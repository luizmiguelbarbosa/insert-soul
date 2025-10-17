#ifndef TRANSITION_H
#define TRANSITION_H

#include "raylib.h"

// Tipos de transição
typedef enum {
    TRANSITION_FADE,
    TRANSITION_ZOOM,
    TRANSITION_DISSOLVE,
    TRANSITION_WIPE,
    TRANSITION_BLINK,
    TRANSITION_BLUR
} TransitionType;

// Estrutura para armazenar informações sobre a transição
typedef struct {
    int width;
    int height;
    float speed;      // A velocidade da transição
    float progress;   // O progresso da transição (0 a 1)
    bool active;      // Se a transição está ativa
    TransitionType type; // Tipo de transição

    unsigned char *dissolveMap; // Usado para a transição de dissolve
    Shader shader; // Usado para a transição de blur
} Transition;

// Funções para as transições
void Transition_Init(Transition *t, int width, int height, float speed, TransitionType type);
bool Transition_UpdateDraw(Transition *t, Texture2D *from, Texture2D *to);
void Transition_Unload(Transition *t);

#endif // TRANSITION_H
