#include "byte2.h"
#include "raylib.h"

// Inclui os cabeçalhos dos módulos do jogo
#include "b2_player.h"
#include "b2_hud.h"
#include "b2_bullet.h"
#include "b2_audio.h" // Isso traz 'extern b2AudioManager'
#include "b2_star.h"
#include "b2_game_state.h"
#include "b2_shop.h"
#include "b2_cutscene.h"
#include "b2_enemy.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

// --- DEFINIÇÕES ---
#define GAME_WIDTH 800
#define GAME_HEIGHT 600

const int STAR_COUNT = 150;

// --- GLOBAIS ---
// REMOVIDO: static AudioManager audioManager; -> A variável agora vive em b2_audio.c

static StarField sideStarField = { 0 };
static Shader crtShader;
static int locResolution;
static int locTime;
static RenderTexture2D target;

// Objetos do jogo
static StarField starField;
static Player player;
static Hud hud;
static BulletManager bulletManager;
static EnemyManager enemyManager;
static ShopScene shop;
static CutsceneScene cutscene;
static GameState currentState;

// Variável de controle de saída
static bool shouldExitGame = false;

// --- FUNÇÕES AUXILIARES (UI) ---
static void DrawShopTransitionUI(EnemyManager *manager) {
    if (!manager->triggerShopReturn) return;

    DrawRectangle(0, 0, GAME_WIDTH, GAME_HEIGHT, Fade(BLACK, 0.9f));

    Rectangle panel = { GAME_WIDTH / 2.0f - 220, GAME_HEIGHT / 2.0f - 110, 440, 220 };
    Color NEON_MAGENTA = (Color){255, 0, 150, 255};
    Color NEON_CYAN = (Color){0, 255, 255, 255};
    Color NEON_GREEN = (Color){0, 255, 0, 255};

    DrawRectangleRounded(panel, 0.15f, 8, Fade(BLACK, 0.7f));

    // DrawRectangleRoundedLines corrigido para Raylib 5.0
    DrawRectangleRoundedLines(panel, 0.15f, 8, Fade(NEON_MAGENTA, 0.8f));
    DrawRectangleRoundedLines(panel, 0.15f, 8, NEON_CYAN);

    const char *waveCompleteText = TextFormat("WAVE %d CONCLUÍDA!", manager->currentWave);
    int titleFontSize = 32;
    int titleX = (int)panel.x + (int)(panel.width - MeasureText(waveCompleteText, titleFontSize)) / 2;
    int titleY = (int)panel.y + 30;

    DrawText(waveCompleteText, titleX + 2, titleY + 2, titleFontSize, Fade(NEON_MAGENTA, 0.5f));
    DrawText(waveCompleteText, titleX, titleY, titleFontSize, NEON_CYAN);

    const char *shopPromptText = "PRONTO PARA A PRÓXIMA MISSÃO?";
    int promptFontSize = 20;
    int promptX = (int)panel.x + (int)(panel.width - MeasureText(shopPromptText, promptFontSize)) / 2;
    DrawText(shopPromptText, promptX, (int)panel.y + 85, promptFontSize, RAYWHITE);

    const char *continueText = "Pressione [E] para visitar a LOJA de MELHORIAS";
    int optionFontSize = 18;
    int optionX1 = (int)panel.x + (int)(panel.width - MeasureText(continueText, optionFontSize)) / 2;
    DrawText(continueText, optionX1, (int)panel.y + 140, optionFontSize, NEON_GREEN);

    const char *skipText = TextFormat("Pressione [F] para CONTINUAR para a WAVE %d", manager->currentWave + 1);
    int optionX2 = (int)panel.x + (int)(panel.width - MeasureText(skipText, optionFontSize)) / 2;
    DrawText(skipText, optionX2, (int)panel.y + 170, optionFontSize, NEON_MAGENTA);
}

static void DrawWaveStartUI(EnemyManager *manager) {
    float t = manager->waveStartTimer;
    if (t <= 0) return;

    DrawRectangle(0, 0, GAME_WIDTH, GAME_HEIGHT, Fade(BLACK, 0.4f));

    float pulseRate = 6.0f;
    float alpha = 0.5f + (sinf((float)GetTime() * pulseRate) + 1.0f) * 0.25f;

    if (t < 1.0f) alpha *= (t / 1.0f);
    alpha = fmaxf(0.0f, fminf(1.0f, alpha));

    const char *text = TextFormat("WAVE %d INICIADA!", manager->currentWave);
    int fontSize = 50;
    int textWidth = MeasureText(text, fontSize);

    DrawText(text, (GAME_WIDTH - textWidth) / 2, GAME_HEIGHT / 2 - fontSize, fontSize, Fade(RAYWHITE, alpha));

    const char *timerText = TextFormat("%.1f", t);
    int timerFontSize = 30;
    int timerTextWidth = MeasureText(timerText, timerFontSize);
    DrawText(timerText, (GAME_WIDTH - timerTextWidth) / 2, GAME_HEIGHT / 2 + 20, timerFontSize, Fade(YELLOW, alpha));
}

// ============================================================================
// --- FUNÇÕES DE INTEGRAÇÃO ---
// ============================================================================

bool ByteSpace_Init(int width, int height) {
    shouldExitGame = false;

    // --- RENDER TEXTURE ---
    target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);

    // --- SHADER CRT ---
    if (FileExists("assets/byte2/shaders/crt22.fs")) {
        crtShader = LoadShader(0, "assets/byte2/shaders/crt22.fs");
    } else {
        crtShader = LoadShader(0, 0);
    }

    locResolution = GetShaderLocation(crtShader, "resolution");
    locTime       = GetShaderLocation(crtShader, "time");

    float initialRes[2] = { (float)width, (float)height };
    SetShaderValue(crtShader, locResolution, initialRes, SHADER_UNIFORM_VEC2);

    // --- INICIALIZAÇÃO ---
    InitStarField(&starField, STAR_COUNT, GAME_WIDTH, GAME_HEIGHT);
    InitStarField(&sideStarField, STAR_COUNT / 2, width, height);

    InitPlayer(&player);
    player.gold = 0;

    InitHud(&hud);
    InitBulletManager(&bulletManager);
    InitEnemyManager(&enemyManager, GAME_WIDTH, GAME_HEIGHT);

    // CORREÇÃO: Usar a variável global b2AudioManager
    InitAudioManager(&b2AudioManager);

    InitShop(&shop, &player, GAME_WIDTH, GAME_HEIGHT);
    InitCutscene(&cutscene);

    // Estado Inicial
    currentState = STATE_CUTSCENE;

    // CORREÇÃO: b2AudioManager
    PlayMusicTrack(&b2AudioManager, MUSIC_CUTSCENE);

    // Posição Jogador
    float player_width_scaled  = player.texture.width * player.scale;
    float player_height_scaled = player.texture.height * player.scale;
    player.position.x = GAME_WIDTH/2 - player_width_scaled/2;
    player.position.y = GAME_HEIGHT - player_height_scaled - 100.0f;

    return true;
}

bool ByteSpace_UpdateDraw(float dt) {

    if (shouldExitGame) return false;

    if (IsKeyPressed(KEY_ESCAPE)) {
        return false;
    }

    // --- ESCALA ---
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    float scaleX = (float)screenW / GAME_WIDTH;
    float scaleY = (float)screenH / GAME_HEIGHT;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;
    int offsetX = (screenW - (int)(GAME_WIDTH * scale)) / 2;
    int offsetY = (screenH - (int)(GAME_HEIGHT * scale)) / 2;

    // --- UPDATE SHADER ---
    float t = GetTime();
    SetShaderValue(crtShader, locTime, &t, SHADER_UNIFORM_FLOAT);
    float res[2] = { (float)screenW, (float)screenH };
    SetShaderValue(crtShader, locResolution, res, SHADER_UNIFORM_VEC2);

    // --- UPDATE LOGIC ---
    // CORREÇÃO: b2AudioManager
    UpdateAudioManager(&b2AudioManager);
    UpdateStarField(&sideStarField, dt);

    switch (currentState) {
        case STATE_CUTSCENE:
            UpdateCutscene(&cutscene, &currentState, dt);
            if (currentState == STATE_SHOP) {
                // CORREÇÃO: b2AudioManager
                StopMusicStream(b2AudioManager.musicCutscene);
                PlayMusicTrack(&b2AudioManager, MUSIC_SHOP);
            }
            break;

        case STATE_ENDING:
            UpdateCutscene(&cutscene, &currentState, dt);
            if (IsKeyPressed(KEY_ENTER)) {
                shouldExitGame = true;
            }
            break;

        case STATE_SHOP:
            UpdateShop(&shop, &player, &starField, &currentState, dt);
            UpdatePlayerBullets(&bulletManager, dt);
            if (currentState == STATE_GAMEPLAY) {
                // CORREÇÃO: b2AudioManager
                StopMusicStream(b2AudioManager.musicShop);
                PlayMusicTrack(&b2AudioManager, MUSIC_GAMEPLAY);
            }
            break;

        case STATE_GAMEPLAY:
            UpdateStarField(&starField, dt);
            UpdateHud(&hud, dt);

            bool isActionPaused = enemyManager.triggerShopReturn || enemyManager.waveStartTimer > 0 || enemyManager.gameOver;

            if (!isActionPaused) {
                // CORREÇÃO: b2AudioManager
                UpdatePlayer(&player, &bulletManager, &b2AudioManager, &hud, dt, GAME_WIDTH, GAME_HEIGHT);
            }
            UpdatePlayerBullets(&bulletManager, dt);
            UpdateEnemies(&enemyManager, dt, GAME_WIDTH, &player.currentLives, &enemyManager.gameOver);

            if (!isActionPaused) {
                // CORREÇÃO: b2AudioManager
                CheckBulletEnemyCollision(&bulletManager, &enemyManager, &player.gold, &b2AudioManager);
            }

            if (enemyManager.triggerShopReturn) {
                if (enemyManager.currentWave == 10) {
                    InitEnding(&cutscene);
                    currentState = STATE_ENDING;
                    enemyManager.triggerShopReturn = false;
                    // CORREÇÃO: b2AudioManager
                    StopMusicStream(b2AudioManager.musicGameplay);
                } else {
                    if (IsKeyPressed(KEY_E)) {
                        currentState = STATE_SHOP;
                        enemyManager.triggerShopReturn = false;
                        // CORREÇÃO: b2AudioManager
                        StopMusicStream(b2AudioManager.musicGameplay);
                        PlayMusicTrack(&b2AudioManager, MUSIC_SHOP);
                    }
                    if (IsKeyPressed(KEY_F)) {
                        enemyManager.triggerShopReturn = false;
                    }
                }
            }

            if (enemyManager.gameOver && IsKeyPressed(KEY_ENTER)) {
                return false;
            }
            break;
    }

    // --- DRAW ---
    BeginTextureMode(target);
        ClearBackground(BLACK);
        switch (currentState) {
            case STATE_CUTSCENE:
            case STATE_ENDING:
                DrawCutscene(&cutscene, GAME_WIDTH, GAME_HEIGHT);
                break;
            case STATE_SHOP:
                DrawShop(&shop, &player, &starField);
                DrawPlayerBullets(&bulletManager);
                break;
            case STATE_GAMEPLAY:
                DrawStarField(&starField);
                const Color NEON_GREEN_LINE = (Color){ 0, 255, 0, 255 };
                DrawRectangle(0, (int)ENEMY_GAME_OVER_LINE_Y, GAME_WIDTH, 2, NEON_GREEN_LINE);
                DrawRectangle(0, (int)ENEMY_GAME_OVER_LINE_Y - 2, GAME_WIDTH, 2, Fade(NEON_GREEN_LINE, 0.4f));
                DrawRectangle(0, (int)ENEMY_GAME_OVER_LINE_Y + 2, GAME_WIDTH, 2, Fade(NEON_GREEN_LINE, 0.4f));
                DrawEnemies(&enemyManager);
                DrawPlayer(&player);
                DrawPlayerBullets(&bulletManager);
                DrawWaveStartUI(&enemyManager);
                if (enemyManager.triggerShopReturn && enemyManager.currentWave < 10) {
                    DrawShopTransitionUI(&enemyManager);
                }
                if (enemyManager.gameOver) {
                    DrawRectangle(0, 0, GAME_WIDTH, GAME_HEIGHT, Fade(BLACK, 0.8f));
                    DrawText("GAME OVER", GAME_WIDTH/2 - MeasureText("GAME OVER", 40)/2, GAME_HEIGHT/2 - 20, 40, RED);
                    DrawText("Pressione [ENTER] para sair", GAME_WIDTH/2 - MeasureText("Pressione [ENTER] para sair", 20)/2, GAME_HEIGHT/2 + 30, 20, WHITE);
                }
                break;
        }
    EndTextureMode();

    BeginDrawing();
        ClearBackground(BLACK);
        if (offsetX > 0 || offsetY > 0) DrawStarField(&sideStarField);

        BeginShaderMode(crtShader);
            DrawTexturePro(target.texture,
                (Rectangle){0, 0, (float)target.texture.width, -(float)target.texture.height},
                (Rectangle){(float)offsetX, (float)offsetY, GAME_WIDTH * scale, GAME_HEIGHT * scale},
                (Vector2){0, 0}, 0.0f, WHITE);
        EndShaderMode();

        if (currentState == STATE_GAMEPLAY && offsetX > 0) {
            DrawHudSide(&hud, true, offsetY, player.energyCharge, player.hasDoubleShot, player.hasShield, player.extraLives, player.currentLives, player.gold);
            DrawHudSide(&hud, false, offsetY, 0.0f, false, false, 0, player.currentLives, player.gold);
        }
    EndDrawing();

    return true;
}

void ByteSpace_Unload(void) {
    UnloadShop(&shop);
    UnloadPlayer(&player);
    UnloadRenderTexture(target);
    UnloadBulletManager(&bulletManager);

    // CORREÇÃO: Unload b2AudioManager
    UnloadAudioManager(&b2AudioManager);

    UnloadShader(crtShader);
}