#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

// --- SEUS INCLUDES (Certifique-se que os .h existem) ---
#include "system.h"
#include "video_player.h"
#include "intro.h"
#include "menu.h"
#include "game.h"        // Lobby
#include "guitar_hero.h" // Minigame

typedef enum {
    STATE_INTRO,
    STATE_MENU,
    STATE_LOADING_LOBBY,
    STATE_LOBBY,
    STATE_GUITAR_HERO
} AppState;

int main(void) {
    int width, height;

    // 1. Inicialização
    System_Init("Insert Your Soul", &width, &height);
    // HideCursor(); // Comente essa linha se quiser ver o mouse para debug

    // 2. Intro
    VideoPlayer vp;
    if (!Intro_Play(&vp, width, height, "assets/frames/intro/frame_%04d.jpg", 793, 60.0f, "assets/audio/intro_audio.wav", 1.5f)) {
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

                // Animação simples de loading
                DrawCircle(width/2 + (int)(sin(GetTime()*5)*50), height/2 + 40, 10, WHITE);
                EndDrawing();

                if (loadingTimer >= 1.0f) { // 1 segundo de loading
                    if (Game_Init(width, height)) {
                        state = STATE_LOBBY;
                    } else {
                        printf("ERRO CRITICO: Falha ao iniciar Game_Init (Lobby).\n");
                        state = STATE_MENU;
                    }
                }
                break;

            // --- LOBBY (CASA) ---
            case STATE_LOBBY:
                // Retorna 0 (nada), 1 (entrar na porta)
                int request = Game_UpdateDraw(deltaTime);

                if (request == 1) {
                    printf("--- TRANSICAO: Lobby -> Guitar Hero ---\n");
                    Game_Unload(); // Libera memória do Lobby

                    // Tenta iniciar o Guitar Hero
                    if (GuitarHero_Init(width, height)) {
                        state = STATE_GUITAR_HERO;
                    } else {
                        // SE FALHAR AQUI, É PORQUE FALTA ARQUIVO NA PASTA ASSETS
                        printf("ERRO: Nao foi possivel iniciar o Guitar Hero.\n");
                        printf("Verifique se 'assets/guitar_musics/teste.mid' existe.\n");

                        // Volta para o Lobby como fallback
                        Game_Init(width, height);
                        state = STATE_LOBBY;
                    }
                }

                if (IsKeyPressed(KEY_ESCAPE)) {
                    Game_Unload();
                    Menu_Init(width, height);
                    state = STATE_MENU;
                }
                break;

            // --- GUITAR HERO ---
            case STATE_GUITAR_HERO:
                GuitarHero_UpdateDraw(deltaTime);

                if (IsKeyPressed(KEY_ESCAPE)) {
                    GuitarHero_Unload();
                    // Ao sair do Guitar Hero, volta para o Lobby
                    if (Game_Init(width, height)) {
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

    // Limpeza final
    if (state == STATE_LOBBY) Game_Unload();
    if (state == STATE_GUITAR_HERO) GuitarHero_Unload();
    Menu_Unload();
    System_Close();

    return 0;
}