#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

// --- SEUS INCLUDES ---
#include "system.h"
#include "video_player.h"
#include "intro.h"
#include "menu.h"
#include "game.h"        // Lobby
#include "guitar_hero.h" // Minigame 1
#include "byte2.h"  // Minigame 2 (Crie este .h se não tiver)

// Definindo as variáveis globais que o game.c usa como 'extern'
bool level1Completed = false;
bool level2Completed = false;

typedef enum {
    STATE_INTRO,
    STATE_MENU,
    STATE_LOADING_LOBBY,
    STATE_LOBBY,
    STATE_GUITAR_HERO,
    STATE_BYTE_SPACE // Novo estado para o jogo 2
} AppState;

int main(void) {
    int width, height;

    // 1. Inicialização do Sistema (Audio e Janela iniciados aqui)
    System_Init("Insert Your Soul", &width, &height);

    // 2. Intro
    VideoPlayer vp;
    if (!Intro_Play(&vp, width, height,
                    "assets/frames/intro/frame_%04d.jpg",
                    793,
                    60.0f,
                    "assets/audio/intro_audio.wav",
                    1.5f)) {
        // Intro pulada ou finalizada
    }

    // 3. Menu
    Menu_Init(width, height);

    AppState state = STATE_MENU;
    bool exitProgram = false;
    float loadingTimer = 0.0f;

    while (!WindowShouldClose() && !exitProgram) {
        float deltaTime = GetFrameTime();
        MenuAction action;

        switch (state) {

            // --- MENU PRINCIPAL ---
            case STATE_MENU:
                action = Menu_UpdateDraw(deltaTime);

                if (action == MENU_ACTION_START || action == MENU_ACTION_CONTINUE) {
                    state = STATE_LOADING_LOBBY;
                    loadingTimer = 0.0f;
                } else if (action == MENU_ACTION_EXIT) {
                    exitProgram = true;
                }
                break;

            // --- TELA DE CARREGAMENTO ---
            case STATE_LOADING_LOBBY:
                loadingTimer += deltaTime;

                BeginDrawing();
                ClearBackground(BLACK);
                DrawText("CARREGANDO...", width/2 - 60, height/2, 20, WHITE);
                // Animação simples
                DrawCircle(width/2 + (int)(sin(GetTime()*5)*50), height/2 + 40, 10, WHITE);
                EndDrawing();

                if (loadingTimer >= 1.0f) {
                    if (Game_Init(width, height)) {
                        state = STATE_LOBBY;
                    } else {
                        printf("ERRO CRITICO: Falha ao iniciar Lobby.\n");
                        state = STATE_MENU;
                    }
                }
                break;

            // --- LOBBY / CASA ---
            case STATE_LOBBY: {
                int request = Game_UpdateDraw(deltaTime);

                // Se request == 1, o jogador apertou 'E' em um arcade disponível
                if (request == 1) {

                    // --- ARCADE 1: GUITAR HERO ---
                    if (selectedArcade == 0) {
                        printf("--- TRANSICAO: Lobby -> Guitar Hero ---\n");
                        Game_Unload(); // Libera RAM do Lobby

                        if (GuitarHero_Init(width, height)) {
                            state = STATE_GUITAR_HERO;
                        } else {
                            printf("ERRO: Falha ao iniciar Guitar Hero.\n");
                            Game_Init(width, height);
                            Game_ResetAfterMiniGame();
                            state = STATE_LOBBY;
                        }
                    }
                    // --- ARCADE 2: BYTE SPACE (NOVO) ---
                    else if (selectedArcade == 1) {
                        printf("--- TRANSICAO: Lobby -> Byte Space ---\n");
                        Game_Unload(); // Libera RAM do Lobby

                        if (ByteSpace_Init(width, height)) {
                            state = STATE_BYTE_SPACE;
                        } else {
                            printf("ERRO: Falha ao iniciar Byte Space.\n");
                            Game_Init(width, height);
                            Game_ResetAfterMiniGame();
                            state = STATE_LOBBY;
                        }
                    }
                }

                if (IsKeyPressed(KEY_ESCAPE)) {
                    Game_Unload();
                    Menu_Init(width, height);
                    state = STATE_MENU;
                }
                break;
            }

            // --- JOGO 1: GUITAR HERO ---
            case STATE_GUITAR_HERO:
                if (!GuitarHero_UpdateDraw(deltaTime)) {
                    // Jogo acabou (Vitória ou Derrota + Enter)
                    GuitarHero_Unload();
                    level1Completed = true; // Marca progresso
                    printf("Nível 1 Completado!\n");

                    // Volta pro Lobby
                    if (Game_Init(width, height)) {
                        Game_ResetAfterMiniGame();
                        state = STATE_LOBBY;
                    } else {
                        Menu_Init(width, height);
                        state = STATE_MENU;
                    }
                }
                break;

            // --- JOGO 2: BYTE SPACE ---
            case STATE_BYTE_SPACE:
                if (!ByteSpace_UpdateDraw(deltaTime)) {
                    // Jogo acabou (Vitória ou Derrota + Enter)
                    ByteSpace_Unload();

                    // Marca progresso do Nível 2
                    level2Completed = true;
                    printf("Nível 2 Completado!\n");

                    // Volta pro Lobby (e libera o Arcade 3 se houver)
                    if (Game_Init(width, height)) {
                        Game_ResetAfterMiniGame();
                        state = STATE_LOBBY;
                    } else {
                        Menu_Init(width, height);
                        state = STATE_MENU;
                    }
                }
                break;

            default:
                break;
        }
    }

    // --- LIMPEZA FINAL ---
    if (state == STATE_LOBBY) Game_Unload();
    if (state == STATE_GUITAR_HERO) GuitarHero_Unload();
    if (state == STATE_BYTE_SPACE) ByteSpace_Unload(); // Limpeza segura

    Menu_Unload();
    System_Close();

    return 0;
}