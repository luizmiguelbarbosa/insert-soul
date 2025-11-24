#ifndef DIALOG_H
#define DIALOG_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_LINES 10
#define MAX_LINE_LENGTH 256

typedef struct {
    bool active;        // diálogo principal ativo
    bool tempActive;    // balão temporário
    const char *lines[MAX_LINES]; // múltiplas linhas
    int lineCount;      // número de linhas
    int currentLine;    // linha atual
    int visibleChars;   // caracteres visíveis da linha atual

    float charTimer;    // timer para digitação
    float charSpeed;    // velocidade de digitação
    float tempTimer;    // timer para balão temporário

    Rectangle box;
} Dialog;

// Funções principais
void Dialog_Init(Dialog *d);
void Dialog_Start(Dialog *d, const char *text);                  // texto simples
void Dialog_StartLines(Dialog *d, const char **lines, int count); // múltiplas linhas
void Dialog_NextLine(Dialog *d);                                  // avança linha
void Dialog_Update(Dialog *d, float deltaTime);
void Dialog_Draw(Dialog *d);

// Funções temporárias (balão rápido)
void Dialog_ShowTemporary(Dialog *d, const char *text, float duration);
void Dialog_UpdateTemporary(Dialog *d, float deltaTime);
void Dialog_DrawTemporary(Dialog *d);

#endif
