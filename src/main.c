#include "raylib.h"
#include "system.h"
#include "video_player.h"
#include "intro.h"
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(void) {
    int width, height;
    System_Init("Insert Your Soul", &width, &height);
    HideCursor();

    VideoPlayer vp;
    if (!Intro_Play(&vp, width, height,
                    "assets/frames/intro/frame_%04d.jpg",
                    793, 60.0f, "assets/audio/intro_audio.wav", 3.0f)) {
        System_Close();
        return -1;
    }

    Menu_Init(width, height);
    bool exitProgram = false;

    while (!WindowShouldClose() && !exitProgram) {
        float deltaTime = GetFrameTime(); // necessário para animação
        MenuAction action = Menu_UpdateDraw(deltaTime);

        switch (action) {
            case MENU_ACTION_START:        printf("New Game clicado!\n"); break;
            case MENU_ACTION_CONTINUE:     printf("Continue clicado!\n"); break;
            case MENU_ACTION_SETTINGS:     printf("Load Settings clicado!\n"); break;
            case MENU_ACTION_CREDITS:      printf("Credits clicado!\n"); break;
            case MENU_ACTION_LANGUAGE:     printf("Language clicado!\n"); break;
            case MENU_ACTION_VOICE:        printf("Voice clicado!\n"); break;
            case MENU_ACTION_EXIT:          exitProgram = true; break;
            default: break;
        }
    }

    Menu_Unload();
    System_Close();
    return 0;
}
