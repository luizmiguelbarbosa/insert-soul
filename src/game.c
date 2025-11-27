#include "game.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include "dialog.h"

// --- DEFINIÇÕES ---
typedef enum {
    CUTSCENE_ENTERING,
    CUTSCENE_DIALOGUE,
    PLAYER_CONTROL
} CutsceneState;

static CutsceneState cutsceneState = CUTSCENE_ENTERING;

#define SPRITE_COLS 8
#define SPRITE_ROWS 6
#define FRAME_TIME 0.1f
#define SCALE 5.0f
#define NUM_ARCADES 3
#define ARCADE_SCALE 0.25f

// -------------------------
//  PLAYER / ANIMAÇÕES
// -------------------------
typedef struct {
    Texture2D texture;
    int cols, rows;
    float frameTime;
} Animation;

typedef struct {
    Vector2 position;
    float speed;
    Vector2 lastDir;
    Animation animIdle;
    Animation animWalk;
    Animation *currentAnim;
    int frame;
    float frameTimer;
} Player;

// -------------------------
//  ARCADES — SISTEMA
// -------------------------
typedef struct {
    Texture2D texBroken;
    Texture2D texFixed;
    Texture2D texCurrent;
    Vector2 position;
    bool canEnter;
} Arcade;

// --- GLOBAIS ---
static Player player;
static Dialog dialog;
static Arcade arcades[NUM_ARCADES];

static bool fading = false;
static float fadeAlpha = 0.0f;
static float fadeSpeed = 2.0f;

// essas flags devem ser definidas/atualizadas no main.c e declaradas como extern aqui:
extern bool level1Completed; // true quando o jogador termina Guitar Hero (nível 1)
extern bool level2Completed; // true quando o jogador termina ByteSpace (nível 2)

// usado pelo main para saber qual arcade foi selecionado
int selectedArcade = -1;

// --- AUXILIAR ---
int GetSpriteRow(Vector2 dir) {
    if (dir.x == 0 && dir.y > 0) return 0;
    if (dir.x < 0 && dir.y == 0) return 1;
    if (dir.x < 0 && dir.y < 0) return 2;
    if (dir.x == 0 && dir.y < 0) return 3;
    if (dir.x > 0 && dir.y < 0) return 4;
    if (dir.x > 0 && dir.y == 0) return 5;
    return 0;
}

// --- INIT ---
bool Game_Init(int width, int height) {

    Dialog_Init(&dialog);

    // -------------------------
    //   PLAYER
    // -------------------------
    player.animIdle.texture = LoadTexture("assets/tiles/player/idle/idle.png");
    player.animIdle.cols = SPRITE_COLS;
    player.animIdle.rows = SPRITE_ROWS;
    player.animIdle.frameTime = 0.2f;

    player.animWalk.texture = LoadTexture("assets/tiles/player/walk/walk.png");
    player.animWalk.cols = SPRITE_COLS;
    player.animWalk.rows = SPRITE_ROWS;
    player.animWalk.frameTime = FRAME_TIME;

    player.position = (Vector2){ -200, height/2.0f };
    player.speed = 250;
    player.frame = 0;
    player.frameTimer = 0;
    player.lastDir = (Vector2){ 1, 0 };
    player.currentAnim = &player.animWalk;

    // -------------------------
    //   ARCADES
    // -------------------------
    float spacing = width / 4.0f;
    float aScale = ARCADE_SCALE;

    // 🎮 ARCADE 0 — Guitar Hero (sempre consertado)
    arcades[0].texFixed   = LoadTexture("assets/arcades/fliperama_guitarhero.png");
    arcades[0].texBroken  = LoadTexture("assets/arcades/fliperama_quebrado_guitarhero.png");
    arcades[0].position   = (Vector2){ spacing*1 - (arcades[0].texFixed.width * aScale)/2.0f, 60 };
    arcades[0].texCurrent = arcades[0].texFixed;
    arcades[0].canEnter   = true;

    // 🎮 ARCADE 1 — ByteSpace (libera após nível 1)
    arcades[1].texFixed   = LoadTexture("assets/arcades/fliperama_byte.png");
    arcades[1].texBroken  = LoadTexture("assets/arcades/fliperama_quebrado_byte.png");
    arcades[1].position   = (Vector2){ spacing*2 - (arcades[1].texFixed.width * aScale)/2.0f, 60 };
    if (level1Completed) {
        arcades[1].texCurrent = arcades[1].texFixed;
        arcades[1].canEnter = true;
    } else {
        arcades[1].texCurrent = arcades[1].texBroken;
        arcades[1].canEnter = false;
    }

    // 🎮 ARCADE 2 — Insert (libera após nível 2)
    arcades[2].texFixed   = LoadTexture("assets/arcades/fliperama_insert.png");
    arcades[2].texBroken  = LoadTexture("assets/arcades/fliperama_quebrado_insert.png");
    arcades[2].position   = (Vector2){ spacing*3 - (arcades[2].texFixed.width * aScale)/2.0f, 60 };
    if (level2Completed) {
        arcades[2].texCurrent = arcades[2].texFixed;
        arcades[2].canEnter = true;
    } else {
        arcades[2].texCurrent = arcades[2].texBroken;
        arcades[2].canEnter = false;
    }

    // -------------------------
    //   ESTADOS DO JOGO
    // -------------------------
    cutsceneState = CUTSCENE_ENTERING;
    fading = false;
    fadeAlpha = 0.0f;
    selectedArcade = -1;

    return true;
}

// --- UPDATE & DRAW ---
int Game_UpdateDraw(float dt) {

    Vector2 move = {0,0};

    int fw = player.currentAnim->texture.width / player.currentAnim->cols;
    int fh = player.currentAnim->texture.height / player.currentAnim->rows;

    float aScale = ARCADE_SCALE;
    int requestLevelChange = 0;

    // -------------------------
    //  CUTSCENE: ENTRANDO
    // -------------------------
    if (cutsceneState == CUTSCENE_ENTERING) {

        player.currentAnim = &player.animWalk;
        player.position.x += 180 * dt;

        float targetX = GetScreenWidth()/2.0f - (fw*SCALE)/2.0f;

        if (player.position.x >= targetX) {
            cutsceneState = CUTSCENE_DIALOGUE;
            player.currentAnim = &player.animIdle;
            Dialog_Start(&dialog,
                "Bem vindo ao Lobby.\n"
                "Use WASD para andar.\n"
                "Interaja com os ARCADES apertando [E].");
        }
    }

    // -------------------------
    //  CUTSCENE: DIÁLOGO
    // -------------------------
    else if (cutsceneState == CUTSCENE_DIALOGUE) {

        Dialog_Update(&dialog, dt);

        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            dialog.active = false;
            cutsceneState = PLAYER_CONTROL;
        }
    }

    // -------------------------
    //  GAMEPLAY
    // -------------------------
    else if (cutsceneState == PLAYER_CONTROL) {

        if (!fading) {

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move.y -= 1;
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move.y += 1;
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) move.x -= 1;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move.x += 1;

            if (move.x != 0 || move.y != 0) {
                player.position.x += move.x * player.speed * dt;
                player.position.y += move.y * player.speed * dt;
                player.lastDir = move;
                player.currentAnim = &player.animWalk;
            } else {
                player.currentAnim = &player.animIdle;
            }
        }

        // Limites horizontais
        if (player.position.x < 0) player.position.x = 0;
        if (player.position.x > GetScreenWidth() - fw*SCALE)
            player.position.x = GetScreenWidth() - fw*SCALE;

        // -------------------------
        //  COLISÃO COM ARCADES
        // -------------------------
        Rectangle pRect = { player.position.x, player.position.y, fw*SCALE, fh*SCALE };

        for (int i = 0; i < NUM_ARCADES; i++) {

            Texture2D tex = arcades[i].texCurrent;

            Rectangle aRect = {
                arcades[i].position.x,
                arcades[i].position.y,
                tex.width * aScale,
                tex.height * aScale
            };

            if (CheckCollisionRecs(pRect, aRect)) {

                // Texto de interação
                DrawText("Aperte [E]",
                         arcades[i].position.x,
                         arcades[i].position.y - 30,
                         20, BLACK);

                if (IsKeyPressed(KEY_E)) {

                    if (arcades[i].canEnter) {
                        fading = true; // inicia transição
                        fadeAlpha = 0.0f;
                        selectedArcade = i; // salva para o main saber qual nível abrir
                    }
                    else {
                        // Arcade quebrado
                        Dialog_Start(&dialog,
                            "O fliperama está quebrado...\n"
                            "Complete o nível anterior para consertá-lo!");
                    }
                }
            }
        }
    }

    // -------------------------
    //  ANIMAÇÃO DO PLAYER
    // -------------------------
    player.frameTimer += dt;
    if (player.frameTimer >= player.currentAnim->frameTime) {
        player.frameTimer = 0;
        player.frame = (player.frame + 1) % player.currentAnim->cols;
    }

    // -------------------------
    //  DRAW
    // -------------------------
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // ---- ARCADES ----
    for (int i = 0; i < NUM_ARCADES; i++) {
        DrawTextureEx(
            arcades[i].texCurrent,
            arcades[i].position,
            0,
            aScale,
            WHITE
        );
    }

    // ---- PLAYER ----
    int row = GetSpriteRow(player.lastDir);
    Rectangle src = { player.frame * fw, row * fh, fw, fh };
    Rectangle dst = { player.position.x, player.position.y, fw*SCALE, fh*SCALE };

    DrawTexturePro(player.currentAnim->texture, src, dst, (Vector2){0,0}, 0, WHITE);

    // ---- DIÁLOGO ----
    if (cutsceneState == CUTSCENE_DIALOGUE) {
        Dialog_Draw(&dialog);
        DrawText("PRESSIONE [ESPAÇO]", 20, GetScreenHeight()-40, 20, DARKGRAY);
    }

    // -------------------------
    //  FADE OUT TRANSIÇÃO
    // -------------------------
    if (fading) {

        fadeAlpha += fadeSpeed * dt;

        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      Fade(BLACK, fadeAlpha));

        if (fadeAlpha >= 1.0f) {
            requestLevelChange = 1;  // sinal para main mudar de cena
        }
    }

    EndDrawing();
    return requestLevelChange;
}

// --- UNLOAD ---
void Game_Unload(void) {
    UnloadTexture(player.animIdle.texture);
    UnloadTexture(player.animWalk.texture);

    for (int i = 0; i < NUM_ARCADES; i++) {
        UnloadTexture(arcades[i].texBroken);
        UnloadTexture(arcades[i].texFixed);
    }
}
