#ifndef BYTE2_H
#define BYTE2_H

#include <stdbool.h>

// Interface pública do minigame BYTE2 — adaptada para o jogo principal.
//
// Implementação (byte2.c) deve prover essas funções:
// - Byte2_Init: inicializa o minigame (carrega assets, configura estados).
// - Byte2_UpdateDraw: chamada a cada frame (update + draw).
// - Byte2_Unload: desaloca recursos ao sair do minigame.
//
// Observação: ajuste os nomes das funções no .c se forem diferentes
// (ou crie um pequeno wrapper que exponha estes nomes).

#ifdef __cplusplus
extern "C" {
#endif

    // Inicializa o minigame byte2. Deve retornar true se tudo foi inicializado com sucesso.
    bool Byte2_Init(int screenWidth, int screenHeight);

    // Atualiza e desenha o minigame — deve ser chamado todo frame enquanto o minigame estiver ativo.
    void Byte2_UpdateDraw(float deltaTime);

    // Descarrega todos os recursos do minigame.
    void Byte2_Unload(void);

#ifdef __cplusplus
}
#endif

#endif // BYTE2_H
