#ifndef GUITAR_HERO_H
#define GUITAR_HERO_H

#include "raylib.h"
#include <stdbool.h>

// Funções de inicialização e loop que o byte2.c chamará
bool GuitarHero_Init(int width, int height);

// ATENÇÃO: Mudou de void para bool.
// Retorna true = continua jogando.
// Retorna false = jogo acabou, voltar para o lobby.
bool GuitarHero_UpdateDraw(float dt);

void GuitarHero_Unload(void);

#endif // GUITAR_HERO_H