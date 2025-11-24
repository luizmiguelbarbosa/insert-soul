#include "dialog.h"
#include <string.h>

void Dialog_Init(Dialog *d) {
    d->active = false;
    d->tempActive = false;
    d->lineCount = 0;
    d->currentLine = 0;
    d->visibleChars = 0;
    d->charTimer = 0;
    d->charSpeed = 0.03f;
    d->tempTimer = 0;

    d->box = (Rectangle){
        40,
        GetScreenHeight() - 160,
        GetScreenWidth() - 80,
        120
    };
}

// -------------------------------------
// Texto simples
void Dialog_Start(Dialog *d, const char *text) {
    d->active = true;
    d->lines[0] = text;
    d->lineCount = 1;
    d->currentLine = 0;
    d->visibleChars = 0;
    d->charTimer = 0;
}

// -------------------------------------
// Múltiplas linhas
void Dialog_StartLines(Dialog *d, const char **lines, int count) {
    d->active = true;
    for (int i = 0; i < count && i < MAX_LINES; i++) {
        d->lines[i] = lines[i];
    }
    d->lineCount = count;
    d->currentLine = 0;
    d->visibleChars = 0;
    d->charTimer = 0;
}

// -------------------------------------
// Avança linha
void Dialog_NextLine(Dialog *d) {
    if (!d->active) return;
    if (d->currentLine + 1 < d->lineCount) {
        d->currentLine++;
        d->visibleChars = 0;
        d->charTimer = 0;
    } else {
        d->active = false; // terminou todas as linhas
    }
}

// -------------------------------------
void Dialog_Update(Dialog *d, float deltaTime) {
    if (!d->active) return;

    d->charTimer += deltaTime;
    const char *text = d->lines[d->currentLine];
    int len = strlen(text);

    if (d->charTimer >= d->charSpeed && d->visibleChars < len) {
        d->charTimer = 0;
        d->visibleChars++;
    }

    // avançar linha ou fechar diálogo ao apertar Enter
    if (d->visibleChars >= len && IsKeyPressed(KEY_ENTER)) {
        Dialog_NextLine(d);
    }
}

// -------------------------------------
void Dialog_Draw(Dialog *d) {
    if (!d->active) return;

    DrawRectangleRec(d->box, (Color){0,0,0,200});
    DrawRectangleLinesEx(d->box, 4, WHITE);

    char buffer[MAX_LINE_LENGTH];
    const char *text = d->lines[d->currentLine];
    strncpy(buffer, text, d->visibleChars);
    buffer[d->visibleChars] = '\0';

    DrawText(buffer, d->box.x + 20, d->box.y + 20, 28, WHITE);
}

// -------------------------------------
// BALÃO TEMPORÁRIO
void Dialog_ShowTemporary(Dialog *d, const char *text, float duration) {
    d->tempActive = true;
    d->lines[0] = text;
    d->lineCount = 1;
    d->currentLine = 0;
    d->visibleChars = strlen(text);
    d->tempTimer = duration;
}

void Dialog_UpdateTemporary(Dialog *d, float deltaTime) {
    if (!d->tempActive) return;
    d->tempTimer -= deltaTime;
    if (d->tempTimer <= 0) d->tempActive = false;
}

void Dialog_DrawTemporary(Dialog *d) {
    if (!d->tempActive) return;
    DrawRectangleRec(d->box, (Color){0,0,0,200});
    DrawRectangleLinesEx(d->box, 4, WHITE);
    DrawText(d->lines[0], d->box.x + 20, d->box.y + 20, 28, WHITE);
}
