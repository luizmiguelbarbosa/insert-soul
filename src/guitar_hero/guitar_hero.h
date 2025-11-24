#ifndef GUITAR_HERO_H
#define GUITAR_HERO_H

#include "raylib.h"
#include <stdbool.h>

// Funções de inicialização e loop que o main.c chamará
bool GuitarHero_Init(int width, int height);
void GuitarHero_UpdateDraw(float dt);
void GuitarHero_Unload(void);

#endif // GUITAR_HERO_H