#include "guitar_hero.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// --- CONFIGURAÇÕES DE LAYOUT (Usadas como base e ajuste dinâmico) ---
#define BASE_WIDTH 1600
#define BASE_HEIGHT 900
#define SPRITE_SIZE 256
#define GUTTER_WIDTH 300
#define NUM_FRETS 5
#define SPEED 450.0f
#define HIT_WINDOW_MS 110.0f
#define MAX_EVENTS 60000
#define MAX_NOTES 10000
#define MAX_PARTICLES 800
#define PARTICLE_LIFE 0.8f

// --- CORES ---
#define CYBER_BG (Color){ 20, 0, 40, 255 }
#define CYBER_PINK (Color){ 255, 0, 110, 255 }
#define CYBER_BLUE (Color){ 0, 243, 255, 255 }
#ifndef CYAN
#define CYAN (Color){ 0, 255, 255, 255 }
#endif

// --- ESTRUTURAS (Copiadas do seu código) ---
typedef struct {
    uint32_t tick;
    int type;
    int note;
    uint32_t tempo;
} RawEvent;

typedef struct {
    float time;
    int fret;
    float sustain;
    bool active;
    bool hit;
} Note;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float life;
    bool active;
} Particle;

typedef enum {
    STATE_START,
    STATE_PLAYING,
    STATE_WIN,
    STATE_LOSE
} GHState;

// --- GLOBAIS (Static para o módulo) ---
static RawEvent events[MAX_EVENTS];
static int eventCount = 0;

static Note notes[MAX_NOTES];
static int noteCount = 0;
static float lastNoteTime = 0.0f;

static Particle particles[MAX_PARTICLES];
static int particlePoolIndex = 0;

static float fret_positions[NUM_FRETS];
static Color fret_colors[NUM_FRETS] = { GREEN, RED, YELLOW, BLUE, ORANGE };
static int key_bindings[NUM_FRETS] = { KEY_A, KEY_S, KEY_D, KEY_F, KEY_G };
static char* key_names[NUM_FRETS] = { "A", "S", "D", "F", "G" };
static float fret_miss_timer[NUM_FRETS] = {0.0f};

static GHState ghState = STATE_START;
static float score = 0;
static int combo = 0;
static float health = 50.0f;
static float audioOffset = 0.0f;

// Assets
static Texture2D background = {0};
static Music song = {0};
static Music vocals = {0};
static bool haveSong = false;
static bool haveVocals = false;

// GIF
static Texture2D *animFrames = NULL;
static int animFramesCount = 0;
static int animCurrentFrame = 0;
static float animTimer = 0.0f;
static bool hasAnim = false;
static const float ANIM_FRAME_SECONDS = 0.08f;

// Variáveis para adaptação de tela
static float current_hit_zone_y = 0.0f;
static float current_game_area_start_x = 0.0f;
static float current_game_area_width = 0.0f;

// --- AUXILIARES MIDI (Copiado do seu código) ---
static uint32_t read_be32(FILE *f) {
    int a = fgetc(f); int b = fgetc(f); int c = fgetc(f); int d = fgetc(f);
    return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | (uint32_t)d;
}
static uint16_t read_be16(FILE *f) {
    int a = fgetc(f); int b = fgetc(f);
    return (uint16_t)(((uint16_t)a << 8) | (uint16_t)b);
}
static uint32_t read_varlen(FILE *f) {
    uint32_t value = 0; int c;
    while ((c = fgetc(f)) != EOF) {
        value = (value << 7) | (c & 0x7F);
        if (!(c & 0x80)) break;
    }
    return value;
}
static int cmp_raw_event(const void *pa, const void *pb) {
    const RawEvent *a = (const RawEvent*)pa;
    const RawEvent *b = (const RawEvent*)pb;
    if (a->tick < b->tick) return -1;
    if (a->tick > b->tick) return 1;
    return 0;
}

// --- PARSER MIDI (Copiado do seu código) ---
int parseMidiFile(const char *path, uint16_t *out_ticksPerQN) {
    FILE *f = fopen(path, "rb");
    if (!f) { printf("ERRO: Arquivo MIDI %s nao encontrado.\n", path); return 0; }

    fseek(f, 4 + 4, SEEK_SET);
    uint16_t format = read_be16(f);
    uint16_t ntrks = read_be16(f);
    *out_ticksPerQN = read_be16(f);

    eventCount = 0;
    for (int t = 0; t < ntrks; t++) {
        char tag[4];
        if (fread(tag, 1, 4, f) != 4) break;
        if (strncmp(tag, "MTrk", 4) != 0) break;
        uint32_t trackLen = read_be32(f);
        long trackStart = ftell(f);
        long trackEnd = trackStart + trackLen;
        int tempEventStart = eventCount;
        bool isGuitarTrack = false;
        uint32_t absTick = 0;
        unsigned char runningStatus = 0;

        while (ftell(f) < trackEnd) {
            uint32_t delta = read_varlen(f);
            absTick += delta;
            unsigned char status = 0;
            int c = fgetc(f);
            if (c & 0x80) { status = c; runningStatus = status; }
            else { status = runningStatus; ungetc(c, f); }

            if (status == 0xFF) {
                int type = fgetc(f);
                uint32_t len = read_varlen(f);
                long dataPos = ftell(f);
                if (type == 0x03) {
                    char trackName[256] = {0};
                    if (len < 255) fread(trackName, 1, len, f);
                    if (strstr(trackName, "PART GUITAR")) isGuitarTrack = true;
                }
                else if (type == 0x51 && len == 3) {
                    int b1 = fgetc(f); int b2 = fgetc(f); int b3 = fgetc(f);
                    if (eventCount < MAX_EVENTS) {
                        events[eventCount].tick = absTick;
                        events[eventCount].type = 0;
                        events[eventCount].tempo = (b1 << 16) | (b2 << 8) | b3;
                        eventCount++;
                    }
                }
                fseek(f, dataPos + len, SEEK_SET);
            }
            else if (status == 0xF0 || status == 0xF7) {
                uint32_t len = read_varlen(f); fseek(f, len, SEEK_CUR);
            }
            else {
                int type = status & 0xF0;
                if (type == 0x90 || type == 0x80) {
                    int note = fgetc(f);
                    int vel = fgetc(f);
                    if (note >= 84 && note <= 88) {
                        if (eventCount < MAX_EVENTS) {
                            events[eventCount].tick = absTick;
                            events[eventCount].note = note;
                            events[eventCount].type = (type == 0x90 && vel > 0) ? 1 : 2;
                            eventCount++;
                        }
                    }
                } else {
                    fgetc(f);
                    if ((type & 0xF0) != 0xC0 && (type & 0xF0) != 0xD0) fgetc(f);
                }
            }
        }
        if (!isGuitarTrack) {
            int writeIdx = tempEventStart;
            for (int i = tempEventStart; i < eventCount; i++) {
                if (events[i].type == 0) { events[writeIdx] = events[i]; writeIdx++; }
            }
            eventCount = writeIdx;
        }
        fseek(f, trackEnd, SEEK_SET);
    }
    fclose(f);
    qsort(events, eventCount, sizeof(RawEvent), cmp_raw_event);
    return 1;
}

void eventsToNotes(uint16_t ticksPerQN) {
    noteCount = 0; lastNoteTime = 0.0f;
    int activeIndex[128];
    for(int i=0; i<128; i++) activeIndex[i] = -1;
    double currentTime = 0.0; uint32_t lastTick = 0; uint32_t tempo = 500000;
    for (int i = 0; i < eventCount; i++) {
        RawEvent *e = &events[i];
        uint32_t delta = e->tick - lastTick;
        currentTime += (double)delta * (double)tempo / ((double)ticksPerQN * 1000000.0);
        lastTick = e->tick;
        if (e->type == 0) tempo = e->tempo;
        else if (e->type == 1) {
            int fret = e->note - 84;
            if (noteCount < MAX_NOTES) {
                notes[noteCount].time = (float)currentTime;
                notes[noteCount].fret = fret;
                notes[noteCount].sustain = 0.0f;
                notes[noteCount].active = true;
                notes[noteCount].hit = false;
                activeIndex[e->note] = noteCount;
                if (notes[noteCount].time > lastNoteTime) lastNoteTime = notes[noteCount].time;
                noteCount++;
            }
        }
        else if (e->type == 2) {
            int idx = activeIndex[e->note];
            if (idx != -1) {
                float dur = (float)currentTime - notes[idx].time;
                if (dur < 0) dur = 0;
                notes[idx].sustain = dur;
                if (notes[idx].time + dur > lastNoteTime) lastNoteTime = notes[idx].time + dur;
                activeIndex[e->note] = -1;
            }
        }
    }
}

// --- VISUALS E PARTICULAS (Copiado do seu código) ---
void DrawScanlines(int w, int h) {
    for (int y = 0; y < h; y += 4) DrawRectangle(0, y, w, 1, Fade(BLACK, 0.2f));
}
void DrawVignette(int w, int h) {
    DrawRing((Vector2){w/2, h/2}, h*0.5f, h*0.9f, 0, 360, 0, Fade(BLACK, 0.0f));
    DrawRing((Vector2){w/2, h/2}, h*0.9f, h*1.2f, 0, 360, 0, BLACK);
}

void SpawnExplosion(Vector2 pos, Color color) {
    for (int i = 0; i < 20; i++) {
        particlePoolIndex = (particlePoolIndex + 1) % MAX_PARTICLES;
        particles[particlePoolIndex].active = true;
        particles[particlePoolIndex].position = pos;
        particles[particlePoolIndex].life = PARTICLE_LIFE;
        particles[particlePoolIndex].velocity = (Vector2){(float)GetRandomValue(-200, 200), (float)GetRandomValue(-250, 50)};
        particles[particlePoolIndex].color = (GetRandomValue(0, 10) < 5) ? color : WHITE;
    }
}
void SpawnMiss(Vector2 pos) {
    for (int i = 0; i < 10; i++) {
        particlePoolIndex = (particlePoolIndex + 1) % MAX_PARTICLES;
        particles[particlePoolIndex].active = true;
        particles[particlePoolIndex].position = (Vector2){pos.x + GetRandomValue(-10,10), pos.y};
        particles[particlePoolIndex].life = 0.5f;
        particles[particlePoolIndex].velocity = (Vector2){(float)GetRandomValue(-50, 50), (float)GetRandomValue(50, 150)};
        particles[particlePoolIndex].color = RED;
    }
}
void SpawnSustainSparks(Vector2 pos, Color color) {
    particlePoolIndex = (particlePoolIndex + 1) % MAX_PARTICLES;
    particles[particlePoolIndex].active = true;
    particles[particlePoolIndex].position = (Vector2){pos.x + GetRandomValue(-5, 5), pos.y};
    particles[particlePoolIndex].life = 0.3f;
    particles[particlePoolIndex].velocity = (Vector2){(float)GetRandomValue(-30, 30), (float)GetRandomValue(-100, -200)};
    particles[particlePoolIndex].color = WHITE;

}
void UpdateParticles(float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        particles[i].life -= dt;
        if (particles[i].life <= 0) { particles[i].active = false; continue; }
        particles[i].position.x += particles[i].velocity.x * dt;
        particles[i].position.y += particles[i].velocity.y * dt;
        particles[i].velocity.y += 400 * dt;
    }
}
void DrawParticles() {
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        DrawRectangleV(particles[i].position, (Vector2){4, 4}, Fade(particles[i].color, particles[i].life));
    }
    EndBlendMode();
}

// --- GIF LOADING (Copiado do seu código) ---
static void LoadGifCorrect(const char *path) {
    if (animFrames != NULL) {
        for (int i = 0; i < animFramesCount; i++) UnloadTexture(animFrames[i]);
        free(animFrames); animFrames = NULL;
    }
    animFramesCount = 0; hasAnim = false;
    int frames = 0;
    Image gif = LoadImageAnim(path, &frames);
    if (gif.data == NULL || frames <= 0) return;
    if (gif.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) ImageFormat(&gif, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    int frameSize = gif.width * gif.height * 4;
    animFrames = (Texture2D*)malloc(sizeof(Texture2D) * frames);
    for (int i = 0; i < frames; i++) {
        Image frameImg = {
            .data = (unsigned char*)gif.data + (i * frameSize),
            .width = gif.width,
            .height = gif.height,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
        };
        animFrames[i] = LoadTextureFromImage(frameImg);
    }
    UnloadImage(gif);
    animFramesCount = frames; hasAnim = true;
}

static void UnloadGifCorrect(void) {
    if (!hasAnim) return;
    for (int i = 0; i < animFramesCount; i++) UnloadTexture(animFrames[i]);
    free(animFrames); animFrames = NULL; hasAnim = false;
}

// --- FUNÇÃO PÚBLICA 1: INICIALIZAÇÃO ---
// Recebe o tamanho da janela do Lobby/Main
bool GuitarHero_Init(int width, int height) {
    score = 0; combo = 0; health = 50.0f; ghState = STATE_START;

    // Reset de globais
    for(int i=0; i<MAX_PARTICLES; i++) particles[i].active = false;
    for(int i=0; i<NUM_FRETS; i++) fret_miss_timer[i] = 0.0f;

    // 1. CARREGA MIDI
    uint16_t ticksPerQN = 480;
    const char* midiPath = "assets/guitar_musics/notes.mid"; // Caminho original

    if (!FileExists(midiPath)) midiPath = "assets/notes.mid"; // Fallback 1
    if (!FileExists(midiPath)) midiPath = "assets/guitar_musics/teste.mid"; // Fallback 2
    if (!FileExists(midiPath)) midiPath = "assets/teste.mid"; // Fallback 3

    if (parseMidiFile(midiPath, &ticksPerQN)) {
        eventsToNotes(ticksPerQN);
    } else {
        printf("ERRO CRITICO: MIDI NAO CARREGADO. Verifique caminhos: assets/guitar_musics/notes.mid\n");
        return false; // Falha na inicialização
    }

    // 2. Cálculo das Posições (Adaptado dinamicamente para o centro da tela)
    current_game_area_start_x = (float)width / 2.0f - (BASE_WIDTH - 2 * GUTTER_WIDTH) / 2.0f; // Ajuste para centralizar
    current_game_area_width = (float)width - (current_game_area_start_x * 2.0f); // Largura do centro

    const float LANE_SPACING = 100.0f;
    const float HIGHWAY_CENTER_X = current_game_area_start_x + current_game_area_width / 2.0f;
    float startX = HIGHWAY_CENTER_X - (((NUM_FRETS - 1) * LANE_SPACING) / 2.0f);

    for (int i = 0; i < NUM_FRETS; i++) fret_positions[i] = startX + (i * LANE_SPACING);

    // 3. Carregamento de Assets
    const char* bgPath = FileExists("assets/background.jpg") ? "assets/background.jpg" : "assets/guitar_musics/background.jpg";
    if(FileExists(bgPath)) background = LoadTexture(bgPath);

    // TENTA CARREGAR GIF
    const char* gifPath = FileExists("assets/guitar.gif") ? "assets/guitar.gif" : "assets/guitar_musics/guitar.gif";
    if(FileExists(gifPath)) LoadGifCorrect(gifPath);

    // 4. Áudio (Sem InitAudioDevice, pois o main já fez isso)
    haveSong = false; haveVocals = false;

    const char* sPath = FileExists("assets/guitar_musics/song.ogg") ? "assets/guitar_musics/song.ogg" : "assets/song.ogg";
    if(FileExists(sPath)) { song = LoadMusicStream(sPath); haveSong = true; }

    const char* vPath = FileExists("assets/guitar_musics/vocals.ogg") ? "assets/guitar_musics/vocals.ogg" : "assets/vocals.ogg";
    if(FileExists(vPath)) { vocals = LoadMusicStream(vPath); haveVocals = true; }

    return true;
}

// --- FUNÇÃO PÚBLICA 2: LOOP DE JOGO ---
void GuitarHero_UpdateDraw(float dt) {
    // Obter tamanho real da janela para desenho responsivo
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    // Recalcular posições relativas (sempre a 75px do fundo)
    current_hit_zone_y = (float)h - 75.0f;
    current_game_area_start_x = (float)w / 2.0f - (BASE_WIDTH - 2 * GUTTER_WIDTH) / 2.0f;
    current_game_area_width = (float)w - (current_game_area_start_x * 2.0f);

    // --- UPDATE LOGIC ---
    if (ghState == STATE_START) {
        if (IsKeyPressed(KEY_SPACE)) {
            ghState = STATE_PLAYING;
            if (haveSong) PlayMusicStream(song);
            if (haveVocals) PlayMusicStream(vocals);
        }
    }
    else if (ghState == STATE_PLAYING) {
        if (haveSong) UpdateMusicStream(song);
        if (haveVocals) UpdateMusicStream(vocals);

        if (hasAnim) {
            animTimer += dt;
            if (animTimer >= ANIM_FRAME_SECONDS) {
                animTimer = 0.0f;
                animCurrentFrame = (animCurrentFrame + 1) % animFramesCount;
            }
        }

        float currentTime = haveSong ? GetMusicTimePlayed(song) + audioOffset : 0.0f;
        if (currentTime > lastNoteTime + 3.0f && noteCount > 0) ghState = STATE_WIN;
        for (int i = 0; i < NUM_FRETS; i++) if (fret_miss_timer[i] > 0) fret_miss_timer[i] -= dt;

        // Logica Miss
        for (int i = 0; i < noteCount; i++) {
            if (!notes[i].active) continue;
            if (notes[i].hit) {
                if (currentTime > notes[i].time + notes[i].sustain) notes[i].active = false;
            } else {
                if ((notes[i].time - currentTime) < -(HIT_WINDOW_MS/1000.0f)) {
                    notes[i].active = false; combo = 0;
                    score -= 50; if(score<0) score=0;
                    fret_miss_timer[notes[i].fret] = 0.2f;
                    health -= 5.0f;
                    if (health <= 0) { health=0; ghState = STATE_LOSE; }
                }
            }
        }
        // Logica Sustain
        for (int i = 0; i < noteCount; i++) {
            if (!notes[i].active || !notes[i].hit) continue;
            if (notes[i].sustain > 0) {
                if (currentTime < notes[i].time + notes[i].sustain) {
                    if (IsKeyDown(key_bindings[notes[i].fret])) {
                        score += 100 * dt;
                        if (GetRandomValue(0,5)==0) SpawnSustainSparks((Vector2){fret_positions[notes[i].fret], current_hit_zone_y}, fret_colors[notes[i].fret]);
                    } else { notes[i].active = false; combo = 0; }
                }
            }
        }
        // Logica Input
        for (int fret = 0; fret < NUM_FRETS; fret++) {
            if (IsKeyPressed(key_bindings[fret])) {
                int bestIdx = -1; float bestDiff = (HIT_WINDOW_MS/1000.0f) + 0.001f;
                for (int i = 0; i < noteCount; i++) {
                    if (!notes[i].active || notes[i].hit || notes[i].fret != fret) continue;
                    float diff = fabsf(notes[i].time - currentTime);
                    if (diff < bestDiff) { bestDiff = diff; bestIdx = i; }
                }
                if (bestIdx != -1) {
                    notes[bestIdx].hit = true; score += 100 + (combo * 10); combo++;
                    SpawnExplosion((Vector2){fret_positions[fret], current_hit_zone_y}, fret_colors[fret]);
                    if (notes[bestIdx].sustain < 0.1f) notes[bestIdx].active = false;
                    health += 5.0f; if(health>100) health=100;
                } else {
                    combo = 0; score -= 50; if(score<0) score=0;
                    SpawnMiss((Vector2){fret_positions[fret], current_hit_zone_y});
                    fret_miss_timer[fret] = 0.3f; health -= 5.0f; if(health<=0) { health=0; ghState = STATE_LOSE; }
                }
            }
        }
        UpdateParticles(dt);
    }

    // --- DRAW ---
    BeginDrawing();

    // Background
    if (background.id > 0) {
        DrawTexturePro(background, (Rectangle){0,0,background.width,background.height}, (Rectangle){0,0,w,h}, (Vector2){0,0}, 0, WHITE);
        DrawRectangle(0,0,w,h, Fade(CYBER_BG, 0.3f));
    } else {
        ClearBackground(CYBER_BG);
    }

    // Laterais
    DrawRectangle(0, 0, GUTTER_WIDTH, h, Fade(BLACK, 0.4f));
    DrawRectangleLines(0, 0, GUTTER_WIDTH, h, Fade(CYBER_BLUE, 0.3f));
    DrawRectangle(w - GUTTER_WIDTH, 0, GUTTER_WIDTH, h, Fade(BLACK, 0.4f));
    DrawRectangleLines(w - GUTTER_WIDTH, 0, GUTTER_WIDTH, h, Fade(CYBER_BLUE, 0.3f));

    // Personagem (GIF)
    if (hasAnim) {
        Texture2D tex = animFrames[animCurrentFrame];
        float scale = SPRITE_SIZE / (float)tex.width;
        // Desenha Esquerda
        DrawTextureEx(tex, (Vector2){(GUTTER_WIDTH-SPRITE_SIZE)/2, (h-SPRITE_SIZE)/2}, 0, scale, WHITE);
        // Desenha Direita (Espelhado)
        Rectangle src = {0, 0, tex.width, tex.height};
        Rectangle dst = {w - GUTTER_WIDTH + (GUTTER_WIDTH-SPRITE_SIZE)/2, (h-SPRITE_SIZE)/2, SPRITE_SIZE, SPRITE_SIZE};
        src.width = -src.width;
        DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0, WHITE);
    }

    // Pista Central
    DrawRectangleGradientV(current_game_area_start_x, 0, current_game_area_width, h, Fade(BLACK,0.0f), Fade(BLACK,0.8f));
    for(int i=0; i<=NUM_FRETS; i++) {
        float x = fret_positions[0] - 50 + (i*100);
        DrawLineV((Vector2){x,0}, (Vector2){x,h}, Fade(CYBER_BLUE, 0.3f));
    }

    // Botões da Base
    for(int i=0; i<NUM_FRETS; i++) {
        float x = fret_positions[i]; Color c = fret_colors[i];
        bool pressed = (ghState == STATE_PLAYING) && IsKeyDown(key_bindings[i]);
        if(pressed) DrawRectangleGradientV(x-50+2, 0, 96, current_hit_zone_y, Fade(c,0), Fade(c,0.3f));
        DrawCircle(x, current_hit_zone_y, 25, Fade(c, pressed?1.0f:0.3f));
        DrawCircleLines(x, current_hit_zone_y, 28, WHITE);
        if(fret_miss_timer[i]>0) DrawText("X", x-10, current_hit_zone_y-15, 30, RED);
    }

    // Notas caindo
    float currentTime = haveSong ? GetMusicTimePlayed(song) + audioOffset : 0.0f;
    for(int i=0; i<noteCount; i++) {
        if(!notes[i].active) continue;
        float dy = (notes[i].time - currentTime) * SPEED;
        float y = current_hit_zone_y - dy;
        float x = fret_positions[notes[i].fret];
        Color c = fret_colors[notes[i].fret];

        if(notes[i].sustain > 0) {
            float tailY = current_hit_zone_y - ((notes[i].time + notes[i].sustain - currentTime) * SPEED);
            float headY = notes[i].hit ? current_hit_zone_y : y;
            if(headY > tailY) DrawRectangle(x-5, tailY, 10, headY-tailY, Fade(c, 0.6f));
        }
        if(!notes[i].hit && y < h+50 && y > -50) {
            DrawCircle(x, y, 20, c);
            DrawCircle(x, y, 12, WHITE);
        }
    }

    DrawParticles();
    DrawVignette(w, h);
    DrawScanlines(w, h);

    // UI Topo
    DrawRectangle(0,0,w, 70, BLACK);
    DrawLine(0, 70, w, 70, CYBER_BLUE);
    DrawText(TextFormat("SCORE: %06d", (int)score), 20, 20, 30, WHITE);
    DrawText(TextFormat("COMBO: %dx", combo), w-200, 20, 30, combo > 30 ? CYBER_PINK : WHITE);

    float barW = 400; float barX = (w-barW)/2;
    DrawRectangleLines(barX, 25, barW, 20, WHITE);
    Color hc = health > 50 ? GREEN : (health > 25 ? YELLOW : RED);
    DrawRectangle(barX+2, 27, (barW-4)*(health/100.0f), 16, hc);

    // Menus Sobrepostos
    if (ghState == STATE_START) {
        DrawRectangle(0, 0, w, h, Fade(BLACK, 0.6f));
        const char* title = "GUITAR HERO";
        DrawText(title, w/2 - MeasureText(title, 80)/2, 150, 80, CYBER_BLUE);
        const char* sub = "CONTROLS";
        DrawText(sub, w/2 - MeasureText(sub, 40)/2, 240, 40, WHITE);

        int startY = 350; int spacingCtrl = 120;
        int totalW = (NUM_FRETS - 1) * spacingCtrl;
        int startCtrlX = (w - totalW) / 2;
        for(int i=0; i<NUM_FRETS; i++) {
            int x = startCtrlX + (i*spacingCtrl);
            DrawCircle(x, startY, 30, fret_colors[i]);
            DrawCircleLines(x, startY, 33, WHITE);
            DrawText(key_names[i], x - MeasureText(key_names[i], 30)/2, startY + 45, 30, WHITE);
        }
        if (((int)(GetTime() * 2)) % 2 == 0) {
            DrawText("PRESS [SPACE] TO START", w/2 - MeasureText("PRESS [SPACE] TO START", 30)/2, h - 150, 30, CYBER_PINK);
        }
    }

    if(ghState == STATE_WIN || ghState == STATE_LOSE) {
        DrawRectangle(0,0,w,h, Fade(BLACK, 0.85f));
        const char* msg = (ghState==STATE_WIN) ? "YOU ROCK!" : "FAILED";
        Color mc = (ghState==STATE_WIN) ? GREEN : RED;
        DrawText(msg, w/2 - MeasureText(msg, 60)/2, h/2 - 30, 60, mc);
        DrawText(TextFormat("FINAL SCORE: %d", (int)score), w/2 - 100, h/2 + 50, 30, WHITE);
    }

    EndDrawing();
}

// --- FUNÇÃO PÚBLICA 3: DESCARREGAMENTO ---
void GuitarHero_Unload(void) {
    if(haveSong) UnloadMusicStream(song);
    if(haveVocals) UnloadMusicStream(vocals);
    UnloadGifCorrect();
    if(background.id > 0) UnloadTexture(background);
}