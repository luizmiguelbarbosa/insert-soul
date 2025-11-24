#ifndef GUITAR_HERO_H
#define GUITAR_HERO_H

#include <stdbool.h>

// Inicializa o jogo (Carrega assets, MIDI, zera variáveis)
// Retorna: true se carregou tudo certo, false se deu erro (ex: não achou o MIDI)
bool GuitarHero_Init(int screenWidth, int screenHeight);

// Atualiza a lógica (notas caindo, input) e desenha UM frame na tela
// O main.c chama isso dentro do loop principal dele
void GuitarHero_UpdateDraw(float deltaTime);

// Descarrega texturas, sons e limpa a memória quando o jogador sai do minigame
void GuitarHero_Unload(void);

#endif