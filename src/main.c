#include <raylib.h>
#include <system.h>
#include <video_player.h>
#include <intro.h>
#include <menu.h>
#include <game.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

typedef enum {
    STATE_INTRO,
    STATE_MENU,
    STATE_LOADING,
    STATE_GAME
} AppState;

int main(void) {
    int width, height;
    System_Init("Insert Your Soul", &width, &height);
    HideCursor();

    // --- INTRO ---
    VideoPlayer vp;
    if (!Intro_Play(&vp, width, height,
                    "assets/frames/intro/frame_%04d.jpg",
                    793, 60.0f, "assets/audio/intro_audio.wav", 1.5f)) {
        System_Close();
        return -1;
    }

    Menu_Init(width, height);

    AppState state = STATE_MENU;
    bool exitProgram = false;
    float loadingTimer = 0.0f;

    while (!WindowShouldClose() && !exitProgram) {
        float deltaTime = GetFrameTime();
        MenuAction action;

        switch (state) {
            // --- MENU ---
            case STATE_MENU:
                action = Menu_UpdateDraw(deltaTime);
                switch (action) {
                    case MENU_ACTION_START:
                    case MENU_ACTION_CONTINUE:
                        state = STATE_LOADING;
                        loadingTimer = 0.0f;
                        break;
                    case MENU_ACTION_SETTINGS:
                        printf("Load Settings clicado!\n");
                        break;
                    case MENU_ACTION_CREDITS:
                        printf("Credits clicado!\n");
                        break;
                    case MENU_ACTION_EXIT:
                        exitProgram = true;
                        break;
                    default:
                        break;
                }
                break;

            // --- LOADING ---
            case STATE_LOADING: {
                loadingTimer += deltaTime;

                BeginDrawing();
                ClearBackground(BLACK);

                // --- Círculo girando no canto inferior direito ---
                float angle = loadingTimer * 360.0f; // velocidade em graus por segundo
                int radius = 20;
                int numDots = 8;
                float dotSize = radius * 0.25f;

                int centerX = width - 80;
                int centerY = height - 80;

                for (int i = 0; i < numDots; i++) {
                    float a = angle + i * (360.0f / numDots);
                    float rad = a * (3.1415926f / 180.0f);
                    int dotX = centerX + (int)(radius * cos(rad));
                    int dotY = centerY + (int)(radius * sin(rad));
                    DrawCircle(dotX, dotY, dotSize, WHITE);
                }

                EndDrawing();

                // Inicia o jogo após 1.5s de loading
                if (loadingTimer >= 1.5f) {
                    if (!Game_Init(width, height)) {
                        printf("Erro ao iniciar o jogo!\n");
                        exitProgram = true;
                    } else {
                        state = STATE_GAME;
                    }
                }
                break;
            }

            // --- GAME ---
            case STATE_GAME:
                Game_UpdateDraw(deltaTime);
                break;

            default:
                break;
        }
    }

    Game_Unload();
    Menu_Unload();
    System_Close();
    return 0;
}
