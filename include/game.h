#ifndef GAME_H
#define GAME_H

#include <raylib.h>
#include <stdbool.h>

/* Inicializa o módulo de jogo.
 * width  - largura da janela (px)
 * height - altura da janela (px)
 * Retorna true se inicialização ocorreu com sucesso. */
bool Game_Init(int width, int height);

/* Atualiza a lógica do jogo e desenha o frame atual.
 * deltaTime - tempo (segundos) desde o último frame. */
void Game_UpdateDraw(float deltaTime);

/* Libera recursos alocados pelo módulo de jogo. */
void Game_Unload(void);

#endif /* GAME_H */
