// system.c
#include "include/system.h"

void System_Init(const char *title, int *width, int *height) {
    getScreenResolution(width, height);

    InitWindow(*width, *height, title);
    InitAudioDevice();
    SetWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_ALWAYS_RUN);
    SetWindowSize(*width, *height);
    SetWindowPosition(0, 0);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    // Limpa tela inicial
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();
}

void System_Close() {
    CloseAudioDevice();
    CloseWindow();
}
