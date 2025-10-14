#include "menu.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h> // Necessário para bool

#define NUM_FRAMES 10
#define BUTTON_COUNT 7

static int screenWidth = 0;
static int screenHeight = 0;

static Texture2D bg;
static Texture2D characterFrames[NUM_FRAMES];
static Music menuMusic;

static int currentFrame = 0;
static float frameTimer = 0.0f;
static float frameTime = 1.0f / 12.0f;

static const char *buttonText[BUTTON_COUNT] = {
    "NEW GAME", "CONTINUE", "SETTINGS", "CREDITS", "LANGUAGE", "VOICE", "EXIT"
};

static Rectangle buttons[BUTTON_COUNT];
static float btnAlpha[BUTTON_COUNT];
static float introAlpha = 0.0f;
static float globalTime = 0.0f;

// --- Tela de confirmação de saída ---
bool Menu_ShowExitConfirmation(int screenWidth, int screenHeight) {
    bool decisionMade = false;
    bool confirmExit = false;

    Rectangle btnYes = {screenWidth/2 - 100, screenHeight/2, 80, 40};
    Rectangle btnNo  = {screenWidth/2 + 20, screenHeight/2, 80, 40};
    Vector2 mouse;

    while (!decisionMade && !WindowShouldClose()) {
        mouse = GetMousePosition();

        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("Deseja realmente sair?", screenWidth/2 - 150, screenHeight/2 - 60, 24, WHITE);

        // Botão SIM
        Color yesColor = CheckCollisionPointRec(mouse, btnYes) ? GREEN : DARKGREEN;
        DrawRectangleRec(btnYes, yesColor);
        DrawText("SIM", btnYes.x + 20, btnYes.y + 10, 20, WHITE);

        // Botão NAO
        Color noColor = CheckCollisionPointRec(mouse, btnNo) ? RED : (Color){139,0,0,255};

        DrawRectangleRec(btnNo, noColor);
        DrawText("NAO", btnNo.x + 10, btnNo.y + 10, 20, WHITE);

        EndDrawing();

        // Interações com teclado
        if (IsKeyPressed(KEY_ENTER)) { confirmExit = true; decisionMade = true; }
        if (IsKeyPressed(KEY_ESCAPE)) { confirmExit = false; decisionMade = true; }

        // Interações com mouse
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, btnYes)) { confirmExit = true; decisionMade = true; }
            if (CheckCollisionPointRec(mouse, btnNo))  { confirmExit = false; decisionMade = true; }
        }
    }

    return confirmExit;
}

// --- Inicialização do menu ---
void Menu_Init(int width, int height) {
    screenWidth = width;
    screenHeight = height;

    bg = LoadTexture("assets/menu/background/bg.jpg");

    char path[64];
    for (int i = 0; i < NUM_FRAMES; i++) {
        sprintf(path, "assets/character/frame%d.png", i + 1);
        characterFrames[i] = LoadTexture(path);
    }

    menuMusic = LoadMusicStream("assets/audio/menu_song.ogg");
    SetMusicVolume(menuMusic, 0.6f);
    PlayMusicStream(menuMusic);

    int buttonWidth = 180;
    int buttonHeight = 40;
    int spacing = 30;
    int totalWidth = BUTTON_COUNT * buttonWidth + (BUTTON_COUNT - 1) * spacing;
    int startX = screenWidth / 2 - totalWidth / 2;
    int posY = screenHeight - 100;

    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttons[i] = (Rectangle){startX + i * (buttonWidth + spacing), posY, buttonWidth, buttonHeight};
        btnAlpha[i] = 0.0f;
    }
}

// --- Atualização e desenho do menu ---
MenuAction Menu_UpdateDraw(float deltaTime) {
    MenuAction action = MENU_ACTION_NONE;
    Vector2 mouse = GetMousePosition();

    UpdateMusicStream(menuMusic);
    globalTime += deltaTime;

    frameTimer += deltaTime;
    if (frameTimer >= frameTime) {
        frameTimer -= frameTime;
        currentFrame = (currentFrame + 1) % NUM_FRAMES;
    }

    if (introAlpha < 1.0f) introAlpha += deltaTime * 0.6f;
    if (introAlpha > 1.0f) introAlpha = 1.0f;

    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (introAlpha >= (float)i / BUTTON_COUNT)
            btnAlpha[i] += deltaTime * 1.2f;
        if (btnAlpha[i] > 1.0f) btnAlpha[i] = 1.0f;
    }

    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexture(bg, 0, 0, Fade(WHITE, introAlpha));

    float bob = sinf(globalTime * 0.8f) * 5.0f;
    int charX = 120;
    int charY = screenHeight / 2 - characterFrames[currentFrame].height / 2 + bob;
    DrawTexture(characterFrames[currentFrame], charX, charY, Fade(WHITE, introAlpha));

    DrawText("Insert Your Synx", 80, 60, 36, Fade(WHITE, introAlpha));

    for (int i = 0; i < BUTTON_COUNT; i++) {
        bool hovered = CheckCollisionPointRec(mouse, buttons[i]);
        Color textColor = hovered ? SKYBLUE : RAYWHITE;
        float alpha = btnAlpha[i] * introAlpha;

        int textWidth = MeasureText(buttonText[i], 22);
        int textX = buttons[i].x + (buttons[i].width - textWidth) / 2;
        int textY = buttons[i].y + (buttons[i].height - 22) / 2;

        DrawText(buttonText[i], textX, textY, 22, Fade(textColor, alpha));

        if (hovered)
            DrawRectangle(textX, buttons[i].y + buttons[i].height - 6, textWidth, 3, Fade(SKYBLUE, alpha));

        if (hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            switch (i) {
                case 0: action = MENU_ACTION_START; break;
                case 1: action = MENU_ACTION_CONTINUE; break;
                case 2: action = MENU_ACTION_SETTINGS; break;
                case 3: action = MENU_ACTION_CREDITS; break;
                case 4: action = MENU_ACTION_LANGUAGE; break;
                case 5: action = MENU_ACTION_VOICE; break;
                case 6: // EXIT
                    if (Menu_ShowExitConfirmation(screenWidth, screenHeight))
                        action = MENU_ACTION_EXIT;
                    break;
            }
        }
    }

    EndDrawing();
    return action;
}

// --- Descarregar recursos ---
void Menu_Unload(void) {
    UnloadTexture(bg);
    for (int i = 0; i < NUM_FRAMES; i++) UnloadTexture(characterFrames[i]);
    StopMusicStream(menuMusic);
    UnloadMusicStream(menuMusic); 
}
