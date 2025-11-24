#ifndef MENU_H
#define MENU_H

#include <raylib.h>
#include <stdbool.h>
#include "video_player.h"

// --- Ações do menu ---
typedef enum {
    MENU_ACTION_NONE,
    MENU_ACTION_START,
    MENU_ACTION_CONTINUE,
    MENU_ACTION_SETTINGS,
    MENU_ACTION_CREDITS,  // NOVO: opção de créditos
    MENU_ACTION_EXIT
} MenuAction;

// --- Funções do menu ---
void Menu_Init(int width, int height);
MenuAction Menu_UpdateDraw(float deltaTime);
void Menu_Unload(void);

// --- Função para mostrar créditos ---
void ShowCredits(void);

#endif
