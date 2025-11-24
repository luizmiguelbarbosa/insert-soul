#include <menu.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <credits.h>
#include "video_player.h"

#define NUM_FRAMES 10
#define BUTTON_COUNT 5

static int screenWidth = 0;
static int screenHeight = 0;

static Texture2D characterFrames[NUM_FRAMES];
static Music menuMusic;
static Sound hoverSound;
static Sound clickSound;

static int currentFrame = 0;
static float frameTimer = 0.0f;
static float frameTime = 1.0f / 12.0f;

static const char *buttonText[BUTTON_COUNT] = {"NEW GAME","CONTINUE","SETTINGS","CREDITS","EXIT"};
static Rectangle buttons[BUTTON_COUNT];
static float btnAlpha[BUTTON_COUNT];
static bool btnHovered[BUTTON_COUNT];

static float introAlpha = 0.0f;
static float globalTime = 0.0f;

static bool showNewGamePopup = false;
static bool showContinuePopup = false;
static bool showExitPopup = false;

static VideoPlayer vpMenu;

static void DrawPopup(Rectangle popup, const char* message, Rectangle btnYes, Rectangle btnNo, Vector2 mouse, int fontSize) {
    DrawRectangleRec(popup, Fade(DARKGRAY, 0.9f));
    DrawRectangleLines(popup.x, popup.y, popup.width, popup.height, WHITE);

    DrawText(message, popup.x + (int)(popup.width * 0.05f), popup.y + (int)(popup.height * 0.1f), fontSize, WHITE);

    Color yesColor = CheckCollisionPointRec(mouse, btnYes) ? GREEN : DARKGREEN;
    Color noColor  = CheckCollisionPointRec(mouse, btnNo)  ? RED : (Color){139,0,0,255};

    DrawRectangleRec(btnYes, yesColor);
    DrawText("YES", btnYes.x + btnYes.width/6, btnYes.y + btnYes.height/8, fontSize, WHITE);

    DrawRectangleRec(btnNo, noColor);
    DrawText("NO", btnNo.x + btnNo.width/6, btnNo.y + btnNo.height/8, fontSize, WHITE);
}

void Menu_Init(int width, int height) {
    screenWidth = width;
    screenHeight = height;

    char path[64];
    for (int i = 0; i < NUM_FRAMES; i++) {
        sprintf(path, "assets/character/frame%d.png", i+1);
        characterFrames[i] = LoadTexture(path);
    }

    menuMusic = LoadMusicStream("assets/audio/menu_song.ogg");
    SetMusicVolume(menuMusic, 0.6f);
    PlayMusicStream(menuMusic);

    hoverSound = LoadSound("assets/audio/hover.wav");
    clickSound = LoadSound("assets/audio/click.wav");

    int spacing = screenWidth / 50;
    int buttonWidth = screenWidth / 7;
    int buttonHeight = screenHeight / 15;
    int totalWidth = BUTTON_COUNT * buttonWidth + (BUTTON_COUNT - 1) * spacing;
    int startX = screenWidth / 2 - totalWidth / 2;
    int posY = screenHeight - buttonHeight - 50;

    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttons[i] = (Rectangle){startX + i*(buttonWidth+spacing), posY, buttonWidth, buttonHeight};
        btnAlpha[i] = 0.0f;
        btnHovered[i] = false;
    }

    VideoPlayer_Init(&vpMenu, "assets/frames/menu/frame_%04d.jpg", 1600, 30.0f, NULL);
}

MenuAction Menu_UpdateDraw(float deltaTime) {
    MenuAction action = MENU_ACTION_NONE;
    Vector2 mouse = GetMousePosition();

    VideoPlayer_Update(&vpMenu, deltaTime);
    UpdateMusicStream(menuMusic);

    if (VideoPlayer_IsFinished(&vpMenu)) {
        VideoPlayer_Reset(&vpMenu); // loop do vídeo do menu
    }

    globalTime += deltaTime;

    frameTimer += deltaTime;
    if (frameTimer >= frameTime) {
        frameTimer -= frameTime;
        currentFrame = (currentFrame + 1) % NUM_FRAMES;
    }

    if (introAlpha < 1.0f) introAlpha += deltaTime * 0.6f;
    if (introAlpha > 1.0f) introAlpha = 1.0f;

    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (introAlpha >= (float)i / BUTTON_COUNT) btnAlpha[i] += deltaTime * 1.2f;
        if (btnAlpha[i] > 1.0f) btnAlpha[i] = 1.0f;
    }

    BeginDrawing();
    ClearBackground(BLACK);

    VideoPlayer_Draw(&vpMenu, 0, 0, screenWidth, screenHeight);

    float bob = sinf(globalTime * 0.8f) * 5.0f;
    int charX = screenWidth / 12;
    int charY = screenHeight / 2 - characterFrames[currentFrame].height/2 + bob;
    DrawTexture(characterFrames[currentFrame], charX, charY, Fade(WHITE,introAlpha));
    DrawText(" ", screenWidth/15, screenHeight/15, screenHeight/25, Fade(WHITE,introAlpha));

    int fontSize = screenHeight / 30;

    for (int i = 0; i < BUTTON_COUNT; i++) {
        bool hovered = CheckCollisionPointRec(mouse, buttons[i]);
        Color textColor = hovered ? SKYBLUE : RAYWHITE;
        float alpha = btnAlpha[i] * introAlpha;

        int textWidth = MeasureText(buttonText[i], fontSize);
        int textX = buttons[i].x + (buttons[i].width - textWidth)/2;
        int textY = buttons[i].y + (buttons[i].height - fontSize)/2;

        DrawText(buttonText[i], textX, textY, fontSize, Fade(textColor,alpha));

        if (hovered)
            DrawRectangle(textX, buttons[i].y + buttons[i].height - fontSize/3, textWidth, fontSize/5, Fade(SKYBLUE,alpha));

        if (hovered && !btnHovered[i]) { PlaySound(hoverSound); btnHovered[i]=true; }
        else if (!hovered && btnHovered[i]) { btnHovered[i]=false; }

        if (hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            PlaySound(clickSound);
            showNewGamePopup = showContinuePopup = showExitPopup = false;

            switch(i){
                case 0: showNewGamePopup=true; break;
                case 1: showContinuePopup=true; break;
                case 2: action = MENU_ACTION_SETTINGS; break;
                case 3: ShowCredits(); break;
                case 4: showExitPopup=true; break;
            }
        }
    }

    int popupFont = screenHeight / 35;

    if (showNewGamePopup) {
        Rectangle popup = {buttons[0].x, buttons[0].y - buttons[0].height - 20, buttons[0].width, buttons[0].height + 30};
        Rectangle btnYes = {popup.x + popup.width*0.1f, popup.y + popup.height*0.5f, popup.width*0.35f, popup.height*0.35f};
        Rectangle btnNo  = {popup.x + popup.width*0.55f, popup.y + popup.height*0.5f, popup.width*0.35f, popup.height*0.35f};
        DrawPopup(popup, "Start a new game?", btnYes, btnNo, mouse, popupFont);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, btnYes)) { PlaySound(clickSound); action = MENU_ACTION_START; showNewGamePopup=false; }
            if (CheckCollisionPointRec(mouse, btnNo))  { PlaySound(clickSound); showNewGamePopup=false; }
        }
    }

    if (showContinuePopup) {
        Rectangle popup = {buttons[1].x, buttons[1].y - buttons[1].height - 20, buttons[1].width, buttons[1].height + 30};
        Rectangle btnYes = {popup.x + popup.width*0.1f, popup.y + popup.height*0.5f, popup.width*0.35f, popup.height*0.35f};
        Rectangle btnNo  = {popup.x + popup.width*0.55f, popup.y + popup.height*0.5f, popup.width*0.35f, popup.height*0.35f};
        DrawPopup(popup, "Load saved game?", btnYes, btnNo, mouse, popupFont);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, btnYes)) { PlaySound(clickSound); action = MENU_ACTION_CONTINUE; showContinuePopup=false; }
            if (CheckCollisionPointRec(mouse, btnNo))  { PlaySound(clickSound); showContinuePopup=false; }
        }
    }

    if (showExitPopup) {
        Rectangle popup = {buttons[4].x, buttons[4].y - buttons[4].height - 20, buttons[4].width, buttons[4].height + 30};
        Rectangle btnYes = {popup.x + popup.width*0.1f, popup.y + popup.height*0.5f, popup.width*0.35f, popup.height*0.35f};
        Rectangle btnNo  = {popup.x + popup.width*0.55f, popup.y + popup.height*0.5f, popup.width*0.35f, popup.height*0.35f};
        DrawPopup(popup, "Quit the game?", btnYes, btnNo, mouse, popupFont);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, btnYes)) { PlaySound(clickSound); action = MENU_ACTION_EXIT; showExitPopup=false; }
            if (CheckCollisionPointRec(mouse, btnNo))  { PlaySound(clickSound); showExitPopup=false; }
        }
    }

    EndDrawing();
    return action;
}

void Menu_Unload(void) {
    for (int i = 0; i < NUM_FRAMES; i++) UnloadTexture(characterFrames[i]);
    StopMusicStream(menuMusic);
    UnloadMusicStream(menuMusic);
    UnloadSound(hoverSound);
    UnloadSound(clickSound);
    VideoPlayer_Unload(&vpMenu);
}
