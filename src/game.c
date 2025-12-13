#include "game.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include "dialog.h"

// --- DEFINIÇÕES ---
typedef enum {
    CUTSCENE_LORE, // Tela preta com o texto de história.
    CUTSCENE_ENTERING,
    CUTSCENE_CONFUSED,
    CUTSCENE_DIALOGUE,
    PLAYER_CONTROL
} CutsceneState;

static CutsceneState cutsceneState = CUTSCENE_LORE;

#define SPRITE_COLS 8
#define SPRITE_ROWS 6
#define FRAME_TIME 0.1f
#define SCALE 5.0f
#define NUM_ARCADES 3
#define ARCADE_SCALE 0.25f

// --- TEXTO COM QUEBRAS DE LINHA MANUAIS ---
// Ajustei levemente as quebras para ficar visualmente agradável centralizado
static const char *loreText =
    "Quando abriu os olhos, tudo ao seu redor ja nao era mais o mundo real.\n"
    "Linhas de codigo flutuavam no ar como poeira luminosa, predios pixelados\n"
    "se erguiam em formatos impossiveis, e o som distante de engrenagens\n"
    "digitais ecoava por todos os lados. Voce havia sido enviado para o\n"
    "Mundo Virtual, um dominio caotico criado por uma IA fora de controle.\n\n"
    "Tudo por um motivo: salvar seu Mike seu marido, sequestrado por uma entidade\n"
    "misteriosa, e resgatar Byte, seu fiel cachorro, capturado enquanto\n"
    "tentava protege-lo. Agora, ambos estavam presos em diferentes camadas\n"
    "desse universo sintetico.\n\n"
    "E havia apenas um caminho para sair dali.\n\n"
    "Para libertar os dois e a si mesmo voce precisara enfrentar o soberano\n"
    "absoluto desse reino digital: Duck, o Super Pato. Uma criatura hibrida de\n"
    "algoritmo e vontade propria, capaz de manipular o proprio codigo do mundo.\n"
    "Ele e o guardiao final, o erro supremo, o glitch que governa tudo.\n\n"
    "Derrote o Super Pato. Restaure o mundo. E traga de volta quem voce ama.";

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

// CUTSCENE CONFUSO TIMERS
static float confusedTimer = 0.0f;
static bool lookRight = true;
static float totalConfused = 0.0f;

// Flags externas
extern bool level1Completed;
extern bool level2Completed;

// Seleção de arcade
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

    // 🎮 ARCADE 0 — Guitar Hero
    arcades[0].texFixed   = LoadTexture("assets/arcades/fliperama_guitarhero.png");
    arcades[0].texBroken  = LoadTexture("assets/arcades/fliperama_quebrado_guitarhero.png");
    arcades[0].position   = (Vector2){ spacing*1 - (arcades[0].texFixed.width * aScale)/2.0f, 60 };
    arcades[0].texCurrent = arcades[0].texFixed;
    arcades[0].canEnter   = true;

    // 🎮 ARCADE 1 — ByteSpace
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

    // 🎮 ARCADE 2 — Insert
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
    cutsceneState = CUTSCENE_LORE;
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
    //  CUTSCENE: LORE
    // -------------------------
    if (cutsceneState == CUTSCENE_LORE) {

        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            cutsceneState = CUTSCENE_ENTERING;
        }

    }
    // -------------------------
    //  CUTSCENE: ENTRANDO
    // -------------------------
    else if (cutsceneState == CUTSCENE_ENTERING) {

        player.currentAnim = &player.animWalk;
        player.position.x += 180 * dt;

        float targetX = GetScreenWidth()/2.0f - (fw*SCALE)/2.0f;

        if (player.position.x >= targetX) {
            cutsceneState = CUTSCENE_CONFUSED;
            player.currentAnim = &player.animIdle;
            player.lastDir = (Vector2){0,1}; // idle down
            confusedTimer = 0.0f;
            lookRight = true;
            totalConfused = 0.0f;
        }
    }
    else if (cutsceneState == CUTSCENE_CONFUSED) {

        confusedTimer += dt;
        totalConfused += dt;

        if (confusedTimer >= 0.5f) {
            confusedTimer = 0;
            if (lookRight) player.lastDir = (Vector2){1,0};
            else           player.lastDir = (Vector2){-1,0};
            lookRight = !lookRight;
        }

        if (totalConfused >= 3.0f) {
            cutsceneState = CUTSCENE_DIALOGUE;
            totalConfused = 0;
            Dialog_Start(&dialog,
                "Bem vindo ao Lobby.\n"
                "Use WASD para andar.\n"
                "Interaja com os ARCADES apertando [E].");
        }
    }
    else if (cutsceneState == CUTSCENE_DIALOGUE) {

        Dialog_Update(&dialog, dt);

        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            dialog.active = false;
            cutsceneState = PLAYER_CONTROL;
        }
    }
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

        if (player.position.x < 0) player.position.x = 0;
        if (player.position.x > GetScreenWidth() - fw*SCALE)
            player.position.x = GetScreenWidth() - fw*SCALE;

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

                DrawText("Aperte [E]",
                         arcades[i].position.x,
                         arcades[i].position.y - 30,
                         20, BLACK);

                if (IsKeyPressed(KEY_E)) {

                    if (arcades[i].canEnter) {
                        fading = true;
                        fadeAlpha = 0.0f;
                        selectedArcade = i;
                    }
                    else {
                        Dialog_Start(&dialog,
                            "O fliperama está quebrado...\n"
                            "Complete o nível anterior para consertá-lo!");
                    }
                }
            }
        }
    }

    player.frameTimer += dt;
    if (player.frameTimer >= player.currentAnim->frameTime) {
        player.frameTimer = 0;
        player.frame = (player.frame + 1) % player.currentAnim->cols;
    }

    BeginDrawing();

    if (cutsceneState == CUTSCENE_LORE) {

        ClearBackground(BLACK);

        float fontSize = 23; // Fonte ligeiramente menor para segurança
        float spacing = 2;

        // 1. Mede o tamanho total do bloco de texto
        Vector2 textSize = MeasureTextEx(GetFontDefault(), loreText, fontSize, spacing);

        // 2. Calcula a posição para centralizar
        Vector2 textPosition;
        textPosition.x = (GetScreenWidth() - textSize.x) / 2.0f;
        textPosition.y = (GetScreenHeight() - textSize.y) / 2.0f;

        // Desenha o texto centralizado
        DrawTextEx(GetFontDefault(), loreText, textPosition, fontSize, spacing, WHITE);

        // Instrução para pular
        const char *continueText = "PRESSIONE [ESPAÇO] PARA CONTINUAR";
        int continueFontSize = 20;

        DrawText(continueText,
                 GetScreenWidth() / 2 - MeasureText(continueText, continueFontSize) / 2,
                 GetScreenHeight() - 40,
                 continueFontSize, DARKGRAY);

    } else {

        ClearBackground(RAYWHITE);

        for (int i = 0; i < NUM_ARCADES; i++) {
            DrawTextureEx(arcades[i].texCurrent, arcades[i].position, 0, aScale, WHITE);
        }

        int row = GetSpriteRow(player.lastDir);
        Rectangle src = { player.frame * fw, row * fh, fw, fh };
        Rectangle dst = { player.position.x, player.position.y, fw*SCALE, fh*SCALE };

        DrawTexturePro(player.currentAnim->texture, src, dst, (Vector2){0,0}, 0, WHITE);

        if (cutsceneState == CUTSCENE_CONFUSED) {
             DrawText("?", player.position.x + fw*SCALE/2, player.position.y - 20, 30, RED);
        }

        if (cutsceneState == CUTSCENE_DIALOGUE) {
            Dialog_Draw(&dialog);
            DrawText("PRESSIONE [ESPAÇO]", 20, GetScreenHeight()-40, 20, DARKGRAY);
        }
    }

    if (fading) {
        fadeAlpha += fadeSpeed * dt;
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, fadeAlpha));
        if (fadeAlpha >= 1.0f) requestLevelChange = 1;
    }

    EndDrawing();
    return requestLevelChange;
}

// --- UNLOAD ---
void Game_Unload(void) {
    Dialog_Unload(&dialog);
    UnloadTexture(player.animIdle.texture);
    UnloadTexture(player.animWalk.texture);

    for (int i = 0; i < NUM_ARCADES; i++) {
        UnloadTexture(arcades[i].texBroken);
        UnloadTexture(arcades[i].texFixed);
    }
}

void Game_ResetAfterMiniGame(void) {
    fading = false;
    fadeAlpha = 0.0f;
    cutsceneState = PLAYER_CONTROL; // Pula a animação de entrada
    selectedArcade = -1;

    // Reseta posição
    player.position.x = 300.0f;

    // Garante que ele olhe para a direita
    player.lastDir = (Vector2){1, 0};
    player.currentAnim = &player.animIdle;
}