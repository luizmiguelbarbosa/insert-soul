#include "guitar_hero.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// --- CONFIGURAÇÕES DE LAYOUT ---
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
#define BASE_HIT_ZONE_OFFSET 75.0f

// --- CORES ---
#define CYBER_BG (Color){ 20, 0, 40, 255 }
#define CYBER_PINK (Color){ 255, 0, 110, 255 }
#define CYBER_BLUE (Color){ 0, 243, 255, 255 }

// --- ESTRUTURAS ---
typedef struct { uint32_t tick; int type; int note; uint32_t tempo; } RawEvent;
typedef struct { float time; int fret; float sustain; bool active; bool hit; } Note;
typedef struct { Vector2 position; Vector2 velocity; Color color; float life; bool active; } Particle;
typedef enum { STATE_START, STATE_PLAYING, STATE_WIN, STATE_LOSE } GHState;

// --- GLOBAIS (STATIC - ESTRUTURA ORIGINAL MANTIDA) ---
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

// Variáveis de estado
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

// Variáveis de Layout dinâmico
static float current_hit_zone_y = 0.0f;
static float current_game_area_start_x = 0.0f;
static float current_game_area_width = 0.0f;

// --- AUXILIARES MIDI/PARSER ---
static uint32_t read_be32(FILE *f) { int a = fgetc(f); int b = fgetc(f); int c = fgetc(f); int d = fgetc(f); return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | (uint32_t)d; }
static uint16_t read_be16(FILE *f) { int a = fgetc(f); int b = fgetc(f); return (uint16_t)(((uint16_t)a << 8) | (uint16_t)b); }
static uint32_t read_varlen(FILE *f) { uint32_t v=0; int c; while ((c = fgetc(f)) != EOF) { v = (v << 7) | (c & 0x7F); if (!(c & 0x80)) break; } return v; }
static int cmp_raw_event(const void *pa, const void *pb) { const RawEvent *a = (const RawEvent*)pa; const RawEvent *b = (const RawEvent*)pb; if (a->tick < b->tick) return -1; if (a->tick > b->tick) return 1; return 1; }
int parseMidiFile(const char *path, uint16_t *out_ticksPerQN) {
    FILE *f = fopen(path, "rb"); if (!f) { printf("ERRO: Arquivo MIDI %s nao encontrado.\n", path); return 0; }
    fseek(f, 4 + 4, SEEK_SET); read_be16(f); uint16_t ntrks = read_be16(f); *out_ticksPerQN = read_be16(f);
    eventCount = 0;
    for (int t = 0; t < ntrks; t++) {
        char tag[4]; if (fread(tag, 1, 4, f) != 4) break; if (strncmp(tag, "MTrk", 4) != 0) break;
        uint32_t len = read_be32(f); long end = ftell(f) + len; int tempStart = eventCount; bool isGuitar = false; uint32_t absTick = 0; unsigned char rs = 0;
        while (ftell(f) < end) {
            uint32_t delta = read_varlen(f); absTick += delta; unsigned char st = 0; int c = fgetc(f);
            if (c & 0x80) { st = c; rs = st; } else { st = rs; ungetc(c, f); }
            if (st == 0xFF) {
                int type = fgetc(f); uint32_t l = read_varlen(f); long dp = ftell(f);
                if (type == 0x03) { char n[256] = {0}; if (l < 255) fread(n, 1, l, f); if (strstr(n, "PART GUITAR")) isGuitar = true; }
                else if (type == 0x51 && l == 3) { int b1 = fgetc(f); int b2 = fgetc(f); int b3 = fgetc(f); if (eventCount < MAX_EVENTS) { events[eventCount] = (RawEvent){absTick, 0, 0, (b1 << 16) | (b2 << 8) | b3}; eventCount++; } }
                fseek(f, dp + l, SEEK_SET);
            } else if (st == 0xF0 || st == 0xF7) { uint32_t l = read_varlen(f); fseek(f, l, SEEK_CUR); }
            else { int type = st & 0xF0; if (type == 0x90 || type == 0x80) { int n = fgetc(f); int v = fgetc(f); if (n >= 84 && n <= 88 && eventCount < MAX_EVENTS) { events[eventCount] = (RawEvent){absTick, (type == 0x90 && v > 0) ? 1 : 2, n, 0}; eventCount++; } } else { fgetc(f); if ((type & 0xF0) != 0xC0 && (type & 0xF0) != 0xD0) fgetc(f); } }
        }
        if (!isGuitar) { int wIdx = tempStart; for (int i = tempStart; i < eventCount; i++) { if (events[i].type == 0) { events[wIdx++] = events[i]; } } eventCount = wIdx; }
        fseek(f, end, SEEK_SET);
    }
    fclose(f);
    qsort(events, eventCount, sizeof(RawEvent), cmp_raw_event);
    return 1;
}

void eventsToNotes(uint16_t ticksPerQN) {
    noteCount = 0; lastNoteTime = 0.0f;
    int activeIndex[128]; for(int i=0;i<128;i++) activeIndex[i]=-1;
    double currentTime=0; uint32_t lastTick=0; uint32_t tempo=500000;
    for(int i=0; i<eventCount; i++){
        RawEvent *e = &events[i]; uint32_t d=e->tick-lastTick; currentTime+=(double)d*(double)tempo/((double)ticksPerQN*1000000.0); lastTick=e->tick;
        if(e->type==0) tempo=e->tempo;
        else if(e->type==1){ int f=e->note-84; if(noteCount<MAX_NOTES){ notes[noteCount]=(Note){(float)currentTime,f,0,true,false}; activeIndex[e->note]=noteCount; if(notes[noteCount].time>lastNoteTime)lastNoteTime=notes[noteCount].time; noteCount++; } }
        else if(e->type==2){ int idx=activeIndex[e->note]; if(idx!=-1){ float dur=(float)currentTime-notes[idx].time; if(dur<0)dur=0; notes[idx].sustain=dur; if(notes[idx].time+dur>lastNoteTime)lastNoteTime=notes[idx].time+dur; activeIndex[e->note]=-1; } }
    }
}

// --- VISUAIS ---
static void DrawScanlines(int screenW, int screenH) { for (int y = 0; y < screenH; y += 4) DrawRectangle(0, y, screenW, 1, Fade(BLACK, 0.2f)); }
// A FUNÇÃO DrawVignette FOI REMOVIDA PARA ELIMINAR O CORTE CIRCULAR.
static void SpawnExplosion(Vector2 pos, Color color) { for (int i = 0; i < 20; i++) { particlePoolIndex = (particlePoolIndex + 1) % MAX_PARTICLES; particles[particlePoolIndex] = (Particle){pos, {(float)GetRandomValue(-200, 200), (float)GetRandomValue(-250, 50)}, (GetRandomValue(0, 10) < 5) ? color : WHITE, PARTICLE_LIFE, true}; } }
static void SpawnMiss(Vector2 pos) { for (int i = 0; i < 10; i++) { particlePoolIndex = (particlePoolIndex + 1) % MAX_PARTICLES; particles[particlePoolIndex] = (Particle){(Vector2){pos.x + GetRandomValue(-10,10), pos.y}, {(float)GetRandomValue(-50, 50), (float)GetRandomValue(50, 150)}, RED, 0.5f, true}; } }
static void SpawnSustainSparks(Vector2 pos, Color color) { for (int i = 0; i < 1; i++) { particlePoolIndex = (particlePoolIndex + 1) % MAX_PARTICLES; particles[particlePoolIndex] = (Particle){(Vector2){pos.x + GetRandomValue(-5, 5), pos.y}, {(float)GetRandomValue(-30, 30), (float)GetRandomValue(-100, -200)}, WHITE, 0.3f, true}; } }
static void UpdateParticles(float dt) { for (int i = 0; i < MAX_PARTICLES; i++) { if (!particles[i].active) continue; particles[i].life -= dt; if (particles[i].life <= 0) { particles[i].active = false; continue; } particles[i].position.x += particles[i].velocity.x * dt; particles[i].position.y += particles[i].velocity.y * dt; particles[i].velocity.y += 400 * dt; } }
static void DrawParticles() { BeginBlendMode(BLEND_ADDITIVE); for (int i = 0; i < MAX_PARTICLES; i++) if (!particles[i].active) continue; else DrawRectangleV(particles[i].position, (Vector2){4, 4}, Fade(particles[i].color, particles[i].life)); EndBlendMode(); }

// --- GIF ---
static void LoadGifCorrect(const char *path) {
    if (animFrames != NULL) {
        for (int i = 0; i < animFramesCount; i++) UnloadTexture(animFrames[i]);
        free(animFrames); animFrames = NULL;
    }

    animFramesCount = 0; hasAnim = false;
    int frames = 0;

    Image gif = LoadImageAnim(path, &frames);

    if (gif.data == NULL || frames <= 0) {
        printf("AVISO GIF: LoadImageAnim falhou para '%s'. Caminho incorreto ou arquivo corrompido.\n", path);
        return;
    }

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
    printf("SUCESSO: GIF '%s' carregado com %d frames.\n", path, frames);
}

static void UnloadGifCorrect(void) {
    if (!hasAnim) return;
    for (int i = 0; i < animFramesCount; i++) UnloadTexture(animFrames[i]);
    free(animFrames); animFrames = NULL; hasAnim = false;
}

// --- FUNÇÃO PÚBLICA 1: INICIALIZAÇÃO (GuitarHero_Init) ---
bool GuitarHero_Init(int width, int height) {
    score = 0; combo = 0; health = 50.0f; ghState = STATE_START;

    for(int i=0; i<MAX_PARTICLES; i++) particles[i].active = false;
    for(int i=0; i<NUM_FRETS; i++) fret_miss_timer[i] = 0.0f;

    // 1. CARREGA MIDI (Com tentativas de fallback)
    uint16_t ticksPerQN = 480;
    const char* midiPath = "assets/guitar_musics/notes.mid";
    if (!FileExists(midiPath)) midiPath = "assets/notes.mid";
    if (!FileExists(midiPath)) midiPath = "assets/guitar_musics/teste.mid";
    if (!FileExists(midiPath)) midiPath = "assets/teste.mid";

    if (parseMidiFile(midiPath, &ticksPerQN)) { eventsToNotes(ticksPerQN); }
    else { return false; }

    // 2. Cálculo das Posições (SETUP)
    current_game_area_start_x = (float)width / 2.0f - (BASE_WIDTH - 2 * GUTTER_WIDTH) / 2.0f;
    current_game_area_width = (float)width - (current_game_area_start_x * 2.0f);

    const float LANE_SPACING = 100.0f;
    const float HIGHWAY_CENTER_X = current_game_area_start_x + current_game_area_width / 2.0f;
    float startX = HIGHWAY_CENTER_X - (((NUM_FRETS - 1) * LANE_SPACING) / 2.0f);

    for (int i = 0; i < NUM_FRETS; i++) fret_positions[i] = startX + (i * LANE_SPACING);

    // 3. Carregamento de Assets
    if(FileExists("assets/background.jpg")) background = LoadTexture("assets/background.jpg");
    else if(FileExists("assets/guitar_musics/background.jpg")) background = LoadTexture("assets/guitar_musics/background.jpg");

    // TENTA CARREGAR GIF
    if(FileExists("assets/guitar.gif")) LoadGifCorrect("assets/guitar.gif");
    else if(FileExists("assets/guitar_musics/guitar.gif")) LoadGifCorrect("assets/guitar_musics/guitar.gif");

    printf("DEBUG: Resultado do carregamento do GIF -> hasAnim: %d, animFramesCount: %d\n", hasAnim, animFramesCount);

    // 4. Áudio
    if (IsAudioDeviceReady()) {
        haveSong = false; haveVocals = false;
        const char* sPath = FileExists("assets/song.ogg") ? "assets/song.ogg" : "assets/guitar_musics/song.ogg";
        if(FileExists(sPath)) { song = LoadMusicStream(sPath); haveSong = true; }
        const char* vPath = FileExists("assets/vocals.ogg") ? "assets/vocals.ogg" : "assets/guitar_musics/vocals.ogg";
        if(FileExists(vPath)) { vocals = LoadMusicStream(vPath); haveVocals = true; }
    }

    return true;
}

// --- FUNÇÃO PÚBLICA 2: LOOP DE JOGO (GuitarHero_UpdateDraw) ---
void GuitarHero_UpdateDraw(float dt) {
    int w = GetScreenWidth();
    int h = GetScreenHeight();

    // Recálculo da HIT ZONE e LAYOUT (para Fullscreen/redimensionamento)
    current_hit_zone_y = (float)h - (BASE_HIT_ZONE_OFFSET * ((float)h / BASE_HEIGHT));
    float current_gutter_width = GUTTER_WIDTH * ((float)w / BASE_WIDTH);
    current_game_area_start_x = current_gutter_width;
    current_game_area_width = w - (current_gutter_width * 2.0f);

    // Recalcula as posições das frets
    const float LANE_SPACING = 100.0f;
    const float HIGHWAY_CENTER_X = current_game_area_start_x + current_game_area_width / 2.0f;
    float startX = HIGHWAY_CENTER_X - (((NUM_FRETS - 1) * LANE_SPACING) / 2.0f);
    for (int i = 0; i < NUM_FRETS; i++) fret_positions[i] = startX + (i * LANE_SPACING);

    const float HIT_ZONE_Y_CURRENT = current_hit_zone_y;
    const float GAME_AREA_START_X_CURRENT = current_game_area_start_x;
    const float GAME_AREA_WIDTH_CURRENT = current_game_area_width;


    // --- UPDATE LOGIC ---
    if (ghState == STATE_START) {
        if (IsKeyPressed(KEY_SPACE)) { ghState = STATE_PLAYING; if (haveSong) PlayMusicStream(song); if (haveVocals) PlayMusicStream(vocals); }
    }
    else if (ghState == STATE_PLAYING) {
        if (haveSong) UpdateMusicStream(song); if (haveVocals) UpdateMusicStream(vocals);

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

        for (int i = 0; i < noteCount; i++) {
            if (!notes[i].active) continue; if (notes[i].hit) { if (currentTime > notes[i].time + notes[i].sustain) notes[i].active = false; }
            else { if ((notes[i].time - currentTime) < -(HIT_WINDOW_MS/1000.0f)) { notes[i].active = false; combo = 0; score -= 50; if(score<0) score=0; fret_miss_timer[notes[i].fret] = 0.2f; health -= 5.0f; if (health <= 0) { health=0; ghState = STATE_LOSE; } } }
        }
        for (int i = 0; i < noteCount; i++) {
            if (!notes[i].active || !notes[i].hit) continue; if (notes[i].sustain > 0) {
                if (currentTime < notes[i].time + notes[i].sustain) { if (IsKeyDown(key_bindings[notes[i].fret])) { score += 100 * dt; if (GetRandomValue(0,5)==0) SpawnSustainSparks((Vector2){fret_positions[notes[i].fret], HIT_ZONE_Y_CURRENT}, fret_colors[notes[i].fret]); } else { notes[i].active = false; combo = 0; } }
            }
        }
        for (int fret = 0; fret < NUM_FRETS; fret++) {
            if (IsKeyPressed(key_bindings[fret])) {
                int bestIdx = -1; float bestDiff = (HIT_WINDOW_MS/1000.0f) + 0.001f;
                for (int i = 0; i < noteCount; i++) {
                    if (!notes[i].active || notes[i].hit || notes[i].fret != fret) continue; float diff = fabsf(notes[i].time - currentTime); if (diff < bestDiff) { bestDiff = diff; bestIdx = i; }
                }
                if (bestIdx != -1) { notes[bestIdx].hit = true; score += 100 + (combo * 10); combo++; SpawnExplosion((Vector2){fret_positions[fret], HIT_ZONE_Y_CURRENT}, fret_colors[fret]); if (notes[bestIdx].sustain < 0.1f) notes[bestIdx].active = false; health += 5.0f; if(health>100) health=100; }
                else { combo = 0; score -= 50; if(score<0) score=0; SpawnMiss((Vector2){fret_positions[fret], HIT_ZONE_Y_CURRENT}); fret_miss_timer[fret] = 0.3f; health -= 5.0f; if(health<=0) { health=0; ghState = STATE_LOSE; } }
            }
        }
        UpdateParticles(dt);
    }

    // --- DRAW ---
    BeginDrawing();

    // Background
    if (background.id > 0) { DrawTexturePro(background, (Rectangle){0,0,background.width,background.height}, (Rectangle){0,0,w,h}, (Vector2){0,0}, 0, WHITE); DrawRectangle(0,0,w,h, Fade(CYBER_BG, 0.3f)); }
    else ClearBackground(CYBER_BG);

    // Valas Laterais
    DrawRectangle(0, 0, GAME_AREA_START_X_CURRENT, h, Fade(BLACK, 0.4f)); DrawRectangleLines(0, 0, GAME_AREA_START_X_CURRENT, h, Fade(CYBER_BLUE, 0.3f));
    DrawRectangle(w - GAME_AREA_START_X_CURRENT, 0, GAME_AREA_START_X_CURRENT, h, Fade(BLACK, 0.4f)); DrawRectangleLines(w - GAME_AREA_START_X_CURRENT, 0, GAME_AREA_START_X_CURRENT, h, Fade(CYBER_BLUE, 0.3f));

    // Personagem (GIF) - AGORA SEM NENHUM CORTE CIRCULAR
    if (hasAnim) {
        Texture2D tex = animFrames[animCurrentFrame];

        float scale = SPRITE_SIZE / (float)tex.width;
        float sprite_display_size = SPRITE_SIZE * scale;

        float pos_y_center = (h - sprite_display_size) / 2.0f;

        // Desenha Esquerda (Centralizado na vala esquerda)
        float pos_x_left = (GAME_AREA_START_X_CURRENT - sprite_display_size) / 2.0f;
        DrawTextureEx(tex, (Vector2){pos_x_left, pos_y_center}, 0, scale, WHITE);

        // Desenha Direita (Espelhado e centralizado na vala direita)
        float pos_x_right = w - GAME_AREA_START_X_CURRENT + pos_x_left;
        Rectangle src_right = {0, 0, (float)tex.width, (float)tex.height};
        Rectangle dst_right = {pos_x_right, pos_y_center, sprite_display_size, sprite_display_size};
        src_right.width = -src_right.width;
        DrawTexturePro(tex, src_right, dst_right, (Vector2){0,0}, 0, WHITE);
    }
    else {
        DrawRectangle((GUTTER_WIDTH-SPRITE_SIZE)/2, (h-SPRITE_SIZE)/2, SPRITE_SIZE, SPRITE_SIZE, RED);
        DrawText("ASSET OFFLINE", (GUTTER_WIDTH-SPRITE_SIZE)/2 + 10, (h-SPRITE_SIZE)/2 + 100, 20, WHITE);
    }

    // Pista Central
    DrawRectangleGradientV(GAME_AREA_START_X_CURRENT, 0, GAME_AREA_WIDTH_CURRENT, h, Fade(BLACK,0.0f), Fade(BLACK,0.8f));
    for(int i=0; i<=NUM_FRETS; i++) { float x = fret_positions[0] - 50 + (i*100); DrawLineV((Vector2){x,0}, (Vector2){x,h}, Fade(CYBER_BLUE, 0.3f)); }

    // Botões da Base e Notas
    for(int i=0; i<NUM_FRETS; i++) { float x = fret_positions[i]; Color c = fret_colors[i]; bool pressed = (ghState==STATE_PLAYING) && IsKeyDown(key_bindings[i]); if(pressed) DrawRectangleGradientV(x-50+2, 0, 96, HIT_ZONE_Y_CURRENT, Fade(c,0), Fade(c,0.3f)); DrawCircle(x, HIT_ZONE_Y_CURRENT, 25, Fade(c, pressed?1.0f:0.3f)); DrawCircleLines(x, HIT_ZONE_Y_CURRENT, 28, WHITE); if(fret_miss_timer[i]>0) DrawText("X", x-10, HIT_ZONE_Y_CURRENT-15, 30, RED); }
    float currentTime = haveSong ? GetMusicTimePlayed(song) + audioOffset : 0.0f;
    for(int i=0; i<noteCount; i++) { if(!notes[i].active) continue; float dy = (notes[i].time - currentTime) * SPEED; float y = HIT_ZONE_Y_CURRENT - dy; float x = fret_positions[notes[i].fret]; Color c = fret_colors[notes[i].fret]; if(notes[i].sustain > 0) { float ty = HIT_ZONE_Y_CURRENT - ((notes[i].time + notes[i].sustain - currentTime) * SPEED); float hy = notes[i].hit ? HIT_ZONE_Y_CURRENT : y; if(hy > ty) DrawRectangle(x-5, ty, 10, hy-ty, Fade(c, 0.6f)); } if(!notes[i].hit && y < h+50 && y > -50) { DrawCircle(x, y, 20, c); DrawCircle(x, y, 12, WHITE); } }

    DrawParticles();
    // DrawVignette(w, h); <--- LINHA ANTERIORMENTE COMENTADA QUE FOI DELETADA DA FUNÇÃO
    DrawScanlines(w, h);

    // UI Topo
    DrawRectangle(0,0,w, 70, BLACK); DrawLine(0, 70, w, 70, CYBER_BLUE); DrawText(TextFormat("SCORE: %06d", (int)score), 20, 20, 30, WHITE); DrawText(TextFormat("COMBO: %dx", combo), w-200, 20, 30, combo > 30 ? CYBER_PINK : WHITE);
    float barW = 400; float barX = (w-barW)/2; DrawRectangleLines(barX, 25, barW, 20, WHITE); Color hc = health > 50 ? GREEN : (health > 25 ? YELLOW : RED); DrawRectangle(barX+2, 27, (barW-4)*(health/100.0f), 16, hc);

    // TELAS DE ESTADO (START / END)
    if (ghState == STATE_START) { DrawRectangle(0, 0, w, h, Fade(BLACK, 0.6f)); DrawText("GUITAR HERO", w/2 - MeasureText("GUITAR HERO",80)/2, 150, 80, CYBER_BLUE); DrawText("CONTROLS", w/2 - MeasureText("CONTROLS", 40)/2, 240, 40, WHITE); int startY = 350; int spacingCtrl = 120; int totalW = (NUM_FRETS - 1) * spacingCtrl; int startCtrlX = (w - totalW) / 2; for(int i=0; i<NUM_FRETS; i++) { int x = startCtrlX + (i*spacingCtrl); DrawCircle(x, startY, 30, fret_colors[i]); DrawCircleLines(x, startY, 33, WHITE); DrawText(key_names[i], x - MeasureText(key_names[i], 30)/2, startY + 45, 30, WHITE); } if (((int)(GetTime() * 2)) % 2 == 0) { DrawText("PRESS [SPACE] TO START", w/2 - MeasureText("PRESS [SPACE] TO START", 30)/2, h - 150, 30, CYBER_PINK); } }
    if(ghState == STATE_WIN || ghState == STATE_LOSE) { DrawRectangle(0,0,w,h, Fade(BLACK, 0.85f)); const char* msg = (ghState==STATE_WIN) ? "YOU ROCK!" : "FAILED"; Color mc = (ghState==STATE_WIN) ? GREEN : RED; DrawText(msg, w/2 - MeasureText(msg, 60)/2, h/2 - 30, 60, mc); DrawText(TextFormat("FINAL SCORE: %d", (int)score), w/2 - 100, h/2 + 50, 30, WHITE); }

    EndDrawing();
}

// --- FUNÇÃO PÚBLICA 3: DESCARREGAMENTO ---
void GuitarHero_Unload(void) {
    if(haveSong) UnloadMusicStream(song);
    if(haveVocals) UnloadMusicStream(vocals);
    UnloadGifCorrect();
    if(background.id > 0) UnloadTexture(background);
    if (IsAudioDeviceReady()) CloseAudioDevice();
}