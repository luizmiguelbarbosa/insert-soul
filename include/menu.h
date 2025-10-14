#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

typedef enum {
    MENU_ACTION_NONE,
    MENU_ACTION_START,
    MENU_ACTION_CONTINUE,
    MENU_ACTION_SETTINGS,
    MENU_ACTION_CREDITS,
    MENU_ACTION_LANGUAGE,
    MENU_ACTION_VOICE,
    MENU_ACTION_EXIT
} MenuAction;

void Menu_Init(int screenWidth, int screenHeight);
MenuAction Menu_UpdateDraw(float deltaTime);
void Menu_Unload(void);

#endif
