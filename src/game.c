#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include "dialog.h"

// -------------------------------------------------------------------
// SISTEMA DE CUTSCENE
// -------------------------------------------------------------------
typedef enum {
    CUTSCENE_ENTERING,
    CUTSCENE_DIALOGUE,
    CUTSCENE_DONE,
    PLAYER_CONTROL,
    PLAYER_WAIT_DIALOGUE
} CutsceneState;

static CutsceneState cutsceneState = CUTSCENE_ENTERING;

// -------------------------------------------------------------------
// SPRITES E ANIMAÇÃO DO PLAYER
// -------------------------------------------------------------------
#define SPRITE_COLS 8
#define SPRITE_ROWS 6
#define FRAME_TIME 0.1f
#define SCALE 5.0f

typedef enum { STATE_IDLE, STATE_WALK } PlayerState;

typedef struct {
    Texture2D texture;
    int cols;
    int rows;
    float frameTime;
} Animation;

typedef struct {
    Vector2 position;
    float speed;
    PlayerState state;
    int frame;
    float frameTimer;
    Vector2 lastDir;
    Animation animIdle;
    Animation animWalk;
    Animation *currentAnim;
} Player;

static Player player;
static Dialog dialog;

// -------------------------------------------------------------------
// PORTAS
// -------------------------------------------------------------------
typedef struct {
    Texture2D texture;
    Vector2 position;
    bool canEnter;
} Door;

#define NUM_DOORS 3
static Door doors[NUM_DOORS];

// -------------------------------------------------------------------
// FADE
// -------------------------------------------------------------------
static bool fading = false;
static float fadeAlpha = 0.0f;
static float fadeSpeed = 2.0f; // Velocidade do fade

// -------------------------------------------------------------------
// FUNÇÃO DE SOM DE MORSE
// -------------------------------------------------------------------
void PlayMorseSound(void) {
    // Aqui você coloca a função de tocar o som de Morse
    // Exemplo: PlaySound(morseSound);
}

// -------------------------------------------------------------------
int GetSpriteRow(Vector2 dir) {
    if (dir.x == 0 && dir.y > 0) return 0; // down
    if (dir.x < 0 && dir.y == 0) return 1; // left
    if (dir.x < 0 && dir.y < 0) return 2;  // up-left
    if (dir.x == 0 && dir.y < 0) return 3; // up
    if (dir.x > 0 && dir.y < 0) return 4;  // up-right
    if (dir.x > 0 && dir.y == 0) return 5; // right
    return 0;
}

// -------------------------------------------------------------------
bool Game_Init(int width, int height) {

    Dialog_Init(&dialog);

    // --- Player ---
    player.animIdle.texture = LoadTexture("assets/tiles/player/idle/idle.png");
    player.animIdle.cols = SPRITE_COLS;
    player.animIdle.rows = SPRITE_ROWS;
    player.animIdle.frameTime = 0.2f;

    player.animWalk.texture = LoadTexture("assets/tiles/player/walk/walk.png");
    player.animWalk.cols = SPRITE_COLS;
    player.animWalk.rows = SPRITE_ROWS;
    player.animWalk.frameTime = FRAME_TIME;

    player.position = (Vector2){ -200, GetScreenHeight()/2 };
    player.speed = 150;
    player.frame = 0;
    player.frameTimer = 0;
    player.lastDir = (Vector2){ 1, 0 }; // andando para direita
    player.currentAnim = &player.animWalk;

    // --- Portas ---
    Texture2D portaAberta = LoadTexture("assets/tiles/casa/porta_aberta.png");
    Texture2D portaFechada = LoadTexture("assets/tiles/casa/porta_fechada.png");
    float doorScale = 3.0f;

    doors[0].texture = portaAberta;
    doors[0].canEnter = true;
    doors[0].position = (Vector2){ width/4 - (portaAberta.width*doorScale)/2, 50 };

    doors[1].texture = portaFechada;
    doors[1].canEnter = false;
    doors[1].position = (Vector2){ width/2 - (portaFechada.width*doorScale)/2, 50 };

    doors[2].texture = portaFechada;
    doors[2].canEnter = false;
    doors[2].position = (Vector2){ 3*width/4 - (portaFechada.width*doorScale)/2, 50 };

    fading = false;
    fadeAlpha = 0.0f;
    cutsceneState = CUTSCENE_ENTERING;

    return true;
}

// -------------------------------------------------------------------
void DrawPlayerShadow(Player *p) {
    int fw = p->currentAnim->texture.width / p->currentAnim->cols;
    int fh = p->currentAnim->texture.height / p->currentAnim->rows;

    float w = fw * SCALE * 0.35f;
    float h = fh * SCALE * 0.10f;

    DrawEllipse(
        p->position.x + (fw * SCALE)/2,
        p->position.y + (fh * SCALE)/2 + 40,
        w/2, h/2,
        (Color){0,0,0,140}
    );
}

// -------------------------------------------------------------------
static void DrawDoors(void) {
    float doorScale = 3.0f;
    for (int i = 0; i < NUM_DOORS; i++) {
        int doorWidth = doors[i].texture.width;
        int doorHeight = doors[i].texture.height;

        Rectangle src = { 0, 0, doorWidth, doorHeight };
        Rectangle dst = { doors[i].position.x, doors[i].position.y, doorWidth*doorScale, doorHeight*doorScale };

        DrawTexturePro(doors[i].texture, src, dst, (Vector2){0,0}, 0, WHITE);
    }
}

// -------------------------------------------------------------------
void Game_UpdateDraw(float dt) {

    Vector2 move = {0,0};
    int fw = player.currentAnim->texture.width / player.currentAnim->cols;
    int fh = player.currentAnim->texture.height / player.currentAnim->rows;

    // -------------------------
    // CUTSCENE
    // -------------------------
    if (cutsceneState == CUTSCENE_ENTERING) {
        player.currentAnim = &player.animWalk;
        player.position.x += 150 * dt;
        float target = GetScreenWidth()/2 - fw*SCALE/2;

        if (player.position.x >= target) {
            player.position.x = target;
            cutsceneState = CUTSCENE_DIALOGUE;
            player.currentAnim = &player.animIdle;
            player.lastDir = (Vector2){0,1};

            Dialog_Start(&dialog,
                "Bem vindo jogador...\n"
                "A sua jornada está prestes a começar."
            );
            PlayMorseSound(); // Morse quando o diálogo iniciar
        }
    }
    else if (cutsceneState == CUTSCENE_DIALOGUE) {
        Dialog_Update(&dialog, dt);
        if (!dialog.active)
            cutsceneState = CUTSCENE_DONE;
    }
    else if (cutsceneState == CUTSCENE_DONE) {
        cutsceneState = PLAYER_CONTROL;
    }

    // -------------------------
    // CONTROLE DO JOGADOR
    // -------------------------
    if (cutsceneState == PLAYER_CONTROL) {

        bool waitingDialogue = dialog.active;

        if (!waitingDialogue && !fading) {
            if (IsKeyDown(KEY_W)) move.y -= 1;
            if (IsKeyDown(KEY_S)) move.y += 1;
            if (IsKeyDown(KEY_A)) move.x -= 1;
            if (IsKeyDown(KEY_D)) move.x += 1;

            if (move.x != 0 || move.y != 0) {
                player.currentAnim = &player.animWalk;
                player.position.x += move.x * player.speed * dt;
                player.position.y += move.y * player.speed * dt;
                player.lastDir = move;
            } else {
                player.currentAnim = &player.animIdle;
            }
        } else {
            player.currentAnim = &player.animIdle;
        }

        // limites de tela
        if (player.position.y < 0) player.position.y = 0;
        float phScaled = fh * SCALE;
        if (player.position.y > GetScreenHeight() - phScaled)
            player.position.y = GetScreenHeight() - phScaled;

        if (player.position.x + fw*SCALE < 0) player.position.x = GetScreenWidth();
        if (player.position.x > GetScreenWidth()) player.position.x = -fw*SCALE;

        // -------------------------
        // Gatilho FADE: acima da porta aberta
        // -------------------------
        float doorScale = 3.0f;
        float triggerHeight = 50.0f; // altura do gatilho acima da porta
        Rectangle playerRect = { player.position.x, player.position.y, fw*SCALE, fh*SCALE };

        Rectangle aboveDoorRect = {
            doors[0].position.x,
            doors[0].position.y - triggerHeight,
            doors[0].texture.width * doorScale,
            triggerHeight
        };

        if (CheckCollisionRecs(playerRect, aboveDoorRect)) {
            fading = true;
        }

        // -------------------------
        // Interação com portas (E)
        // -------------------------
        for (int i = 0; i < NUM_DOORS; i++) {
            Rectangle doorRect = {doors[i].position.x, doors[i].position.y,
                                  doors[i].texture.width*doorScale, doors[i].texture.height*doorScale};
            if (CheckCollisionRecs(playerRect, doorRect) && IsKeyPressed(KEY_E)) {
                player.lastDir = (Vector2){1,0};
                if (doors[i].canEnter) {
                    Dialog_Start(&dialog, "Acho que devo entrar nessa porta.");
                    PlayMorseSound(); // Morse ao iniciar o diálogo
                } else {
                    Dialog_Start(&dialog, "Esta porta está fechada!");
                    PlayMorseSound(); // Morse ao iniciar o diálogo
                }
            }
        }
    }

    // -------------------------
    // ANIMAÇÃO PLAYER
    // -------------------------
    player.frameTimer += dt;
    if (player.frameTimer >= player.currentAnim->frameTime) {
        player.frameTimer = 0;
        player.frame = (player.frame + 1) % SPRITE_COLS;
    }

    int row = GetSpriteRow(player.lastDir);

    // -------------------------
    // DESENHO FINAL
    // -------------------------
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawPlayerShadow(&player);
    DrawDoors();

    Rectangle src = { player.frame * fw, row * fh, fw, fh };
    Rectangle dst = { player.position.x, player.position.y, fw*SCALE, fh*SCALE };
    DrawTexturePro(player.currentAnim->texture, src, dst, (Vector2){0,0}, 0, WHITE);

    Dialog_Draw(&dialog);

    // -------------------------
    // FADE BRANCO
    // -------------------------
    if (fading) {
        fadeAlpha += fadeSpeed * dt;
        if (fadeAlpha > 1.0f) fadeAlpha = 1.0f;
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){255,255,255,(unsigned char)(fadeAlpha*255)});
    }

    EndDrawing();
}

// -------------------------------------------------------------------
void Game_Unload(void) {
    UnloadTexture(player.animIdle.texture);
    UnloadTexture(player.animWalk.texture);
    for (int i = 0; i < NUM_DOORS; i++)
        UnloadTexture(doors[i].texture);
}
