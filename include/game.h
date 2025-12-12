#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

// Variáveis globais compartilhadas (definidas no main ou game.c)
extern int selectedArcade;

// Funções
bool Game_Init(int width, int height);
int Game_UpdateDraw(float dt); // Retorna 1 se pediu troca de nível
void Game_Unload(void);
void Game_ResetAfterMiniGame(void); // <--- Nova função

#endif