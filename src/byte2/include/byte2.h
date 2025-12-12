#ifndef BYTE2_H
#define BYTE2_H

#include <stdbool.h>

// Inicializa o minigame Byte Space
// Retorna true se carregou com sucesso
bool ByteSpace_Init(int width, int height);

// Atualiza e desenha o frame
// Retorna true: Jogo continua rodando
// Retorna false: Jogo acabou (venceu ou perdeu), voltar para o lobby
bool ByteSpace_UpdateDraw(float dt);

// Descarrega texturas e sons para liberar memória
void ByteSpace_Unload(void);

#endif // BYTE2_H