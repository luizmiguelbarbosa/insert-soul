// game.h
#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

// Inicializa o jogo
bool Game_Init(int screenWidth, int screenHeight);

// MUDANÇA AQUI: Agora retorna um int (0 = nada, 1 = ir para guitar hero)
int Game_UpdateDraw(float deltaTime);

// Descarrega tudo
void Game_Unload(void);

#endif