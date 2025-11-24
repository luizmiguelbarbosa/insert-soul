#include "game.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include "dialog.h" // Certifique-se de ter esse .h ou remover as referências

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
#define NUM_DOORS 3

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

typedef struct {
    Texture2D texture;
    Vector2 position;
    bool canEnter;
} Door;

// --- GLOBAIS ---
static Player player;
static Dialog dialog; // Se não tiver dialog.h, remova isso
static Door doors[NUM_DOORS];

static bool fading = false;
static float fadeAlpha = 0.0f;
static float fadeSpeed = 2.0f;

// --- FUNÇÕES AUXILIARES ---
int GetSpriteRow(Vector2 dir) {
    if (dir.x == 0 && dir.y > 0) return 0; // Baixo
    if (dir.x < 0 && dir.y == 0) return 1; // Esquerda
    if (dir.x < 0 && dir.y < 0) return 2;  // Cima-Esq
    if (dir.x == 0 && dir.y < 0) return 3; // Cima
    if (dir.x > 0 && dir.y < 0) return 4;  // Cima-Dir
    if (dir.x > 0 && dir.y == 0) return 5; // Direita
    return 0;
}

// --- INIT ---
bool Game_Init(int width, int height) {
    Dialog_Init(&dialog); // Se tiver dialog system

    // Carregar Texturas (Verifique os caminhos!)
    player.animIdle.texture = LoadTexture("assets/tiles/player/idle/idle.png");
    player.animIdle.cols = SPRITE_COLS; player.animIdle.rows = SPRITE_ROWS; player.animIdle.frameTime = 0.2f;

    player.animWalk.texture = LoadTexture("assets/tiles/player/walk/walk.png");
    player.animWalk.cols = SPRITE_COLS; player.animWalk.rows = SPRITE_ROWS; player.animWalk.frameTime = FRAME_TIME;

    // Configurar Player
    player.position = (Vector2){ -200, height/2.0f }; // Começa fora da tela
    player.speed = 250;
    player.frame = 0; player.frameTimer = 0;
    player.lastDir = (Vector2){ 1, 0 };
    player.currentAnim = &player.animWalk;

    // Configurar Portas
    Texture2D pAberta = LoadTexture("assets/tiles/casa/porta_aberta.png");
    Texture2D pFechada = LoadTexture("assets/tiles/casa/porta_fechada.png");
    float dScale = 3.0f;
    float spacing = width / 4.0f;

    // Porta 0 = Guitar Hero (Aberta)
    doors[0].texture = pAberta; doors[0].canEnter = true;
    doors[0].position = (Vector2){ spacing * 1 - (pAberta.width*dScale)/2, 50 };

    // Outras portas
    doors[1].texture = pFechada; doors[1].canEnter = false;
    doors[1].position = (Vector2){ spacing * 2 - (pFechada.width*dScale)/2, 50 };

    doors[2].texture = pFechada; doors[2].canEnter = false;
    doors[2].position = (Vector2){ spacing * 3 - (pFechada.width*dScale)/2, 50 };

    // Resetar estado
    cutsceneState = CUTSCENE_ENTERING;
    fading = false;
    fadeAlpha = 0.0f;

    return true;
}

// --- UPDATE & DRAW ---
int Game_UpdateDraw(float dt) {
    Vector2 move = {0,0};
    int fw = player.currentAnim->texture.width / player.currentAnim->cols;
    int fh = player.currentAnim->texture.height / player.currentAnim->rows;
    float dScale = 3.0f;
    int requestLevelChange = 0; // 0 = Fica aqui, 1 = Muda de fase

    // 1. Cutscene: Entrada
    if (cutsceneState == CUTSCENE_ENTERING) {
        player.currentAnim = &player.animWalk;
        player.position.x += 180 * dt;

        // Vai até o meio da tela
        if (player.position.x >= GetScreenWidth()/2.0f - (fw*SCALE)/2.0f) {
            cutsceneState = CUTSCENE_DIALOGUE;
            player.currentAnim = &player.animIdle;
            Dialog_Start(&dialog, "Bem vindo ao Lobby.\nUse WASD para andar.\nEntre na porta ABERTA.");
        }
    }
    // 2. Cutscene: Diálogo
    else if (cutsceneState == CUTSCENE_DIALOGUE) {
        Dialog_Update(&dialog, dt);
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            dialog.active = false;
            cutsceneState = PLAYER_CONTROL;
        }
    }
    // 3. Gameplay
    else if (cutsceneState == PLAYER_CONTROL) {
        if (!fading) {
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    move.y -= 1;
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  move.y += 1;
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  move.x -= 1;
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

        // Limites da tela
        if (player.position.x < 0) player.position.x = 0;
        if (player.position.x > GetScreenWidth() - fw*SCALE) player.position.x = GetScreenWidth() - fw*SCALE;

        // Checar Portas
        if (!fading) {
            Rectangle pRect = { player.position.x, player.position.y, fw*SCALE, fh*SCALE };
            for (int i = 0; i < NUM_DOORS; i++) {
                Rectangle dRect = { doors[i].position.x, doors[i].position.y, doors[i].texture.width * dScale, doors[i].texture.height * dScale };

                if (CheckCollisionRecs(pRect, dRect)) {
                    DrawText("Aperte [E]", doors[i].position.x, doors[i].position.y - 30, 20, BLACK);

                    if (IsKeyPressed(KEY_E)) {
                        if (doors[i].canEnter) {
                            fading = true; // Inicia a transição
                        } else {
                            // Porta trancada
                        }
                    }
                }
            }
        }
    }

    // Animação Frame
    player.frameTimer += dt;
    if (player.frameTimer >= player.currentAnim->frameTime) {
        player.frameTimer = 0;
        player.frame = (player.frame + 1) % player.currentAnim->cols;
    }

    // --- DRAW ---
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Portas
    for (int i = 0; i < NUM_DOORS; i++) {
        DrawTextureEx(doors[i].texture, doors[i].position, 0.0f, dScale, WHITE);
    }

    // Player
    int row = GetSpriteRow(player.lastDir);
    Rectangle src = { player.frame * fw, row * fh, fw, fh };
    Rectangle dst = { player.position.x, player.position.y, fw*SCALE, fh*SCALE };
    DrawTexturePro(player.currentAnim->texture, src, dst, (Vector2){0,0}, 0, WHITE);

    // HUD / Diálogo
    if (cutsceneState == CUTSCENE_DIALOGUE) {
        Dialog_Draw(&dialog);
        DrawText("PRESSIONE [ESPACO]", 20, GetScreenHeight() - 40, 20, DARKGRAY);
    }

    // Fade Out (Transição de Nível)
    if (fading) {
        fadeAlpha += fadeSpeed * dt;
        DrawRectangle(0,0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, fadeAlpha));

        if (fadeAlpha >= 1.0f) {
            requestLevelChange = 1; // <--- AQUI MANDA O SINAL PRO MAIN.C
        }
    }

    EndDrawing();
    return requestLevelChange;
}

void Game_Unload(void) {
    UnloadTexture(player.animIdle.texture);
    UnloadTexture(player.animWalk.texture);
    for(int i=0; i<NUM_DOORS; i++) UnloadTexture(doors[i].texture);
}