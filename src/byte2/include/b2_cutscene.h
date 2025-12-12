#ifndef B2_CUTSCENE_H
#define B2_CUTSCENE_H

#include "raylib.h"
#include "b2_game_state.h" // Garante que GameState seja conhecido

#define MAX_COMIC_PANELS 5

// Estrutura para uma página de texto da intro
typedef struct {
    char text[64];
    float duration;
} CutscenePage;

typedef struct {
    // Controle de Estado
    bool isEnding;      // true = final do jogo, false = intro

    // !!! CORREÇÃO: A variável que faltava !!!
    bool finished;

    // Dados da Intro (Texto Neon)
    CutscenePage pages[2];
    int currentPage;
    float currentTimer;
    float textTimer;
    int visibleChars;
    bool isFadingOut;
    float titleAlpha;
    bool showTitle;

    // Dados do Final (Quadrinhos)
    Texture2D endingImages[MAX_COMIC_PANELS];
    int endingImageIndex;
    // Opcional: timers para troca automática se quiser
    float panelDurations[MAX_COMIC_PANELS];
    float timer;
    int currentPanel; // Usado se for automático
    Texture2D panelTextures[MAX_COMIC_PANELS]; // Usado na intro antiga se precisar

} CutsceneScene;

// --- FUNÇÕES ---
void InitCutscene(CutsceneScene *cs);
void InitEnding(CutsceneScene *cs);
void UpdateCutscene(CutsceneScene *cs, GameState *state, float dt);
void DrawCutscene(CutsceneScene *cs, int screenWidth, int screenHeight);

#endif // B2_CUTSCENE_H