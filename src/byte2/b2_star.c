#include "b2_star.h"
#include "raylib.h"
#include "raymath.h" // Inclui funções matemáticas úteis, como Clamp
#include <stdlib.h>

// --- FUNÇÃO DE INICIALIZAÇÃO DO CAMPO DE ESTRELAS ---
// Aloca memória e configura as estrelas individuais
void InitStarField(StarField *field, int count, int screenWidth, int screenHeight) {
    field->count = count; // Define o número total de estrelas
    field->screenWidth = screenWidth;
    field->screenHeight = screenHeight;

    // Aloca memória para o array de estrelas (usa a função MemAlloc do raylib)
    field->stars = (Star *)MemAlloc(sizeof(Star) * count);

    // Loop para inicializar cada estrela
    for (int i = 0; i < count; i++) {
        // Posição: Aleatória dentro das dimensões da tela
        field->stars[i].position = (Vector2){ (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
        field->stars[i].color = WHITE;

        // Tamanho: Aleatório entre o mínimo e o máximo definidos (multiplica por 10 para usar GetRandomValue com floats)
        field->stars[i].size = (float)GetRandomValue(MIN_STAR_SIZE * 10, MAX_STAR_SIZE * 10) / 10.0f;

        // --- LÓGICA DE PISCA-PISCA (BLINKING) ---
        // Timer inicial: Aleatório, para que as estrelas comecem a piscar em momentos diferentes
        field->stars[i].blinkTimer = (float)GetRandomValue(0, 100) / 100.0f;
        // Duração do ciclo de pisca-pisca (quanto tempo leva para mudar de opacidade)
        field->stars[i].blinkDuration = (float)GetRandomValue(50, 200) / 100.0f; // Entre 0.5 e 2.0 segundos

        field->stars[i].currentAlpha = 1.0f; // Começa totalmente visível
        field->stars[i].isFadingIn = true;   // Começa no modo "claro"
    }
}

// --- FUNÇÃO DE ATUALIZAÇÃO DO CAMPO DE ESTRELAS ---
// Move as estrelas e gerencia o efeito de pisca-pisca
void UpdateStarField(StarField *field, float deltaTime) {
    for (int i = 0; i < field->count; i++) {

        // 1. Movimento: As estrelas se movem para baixo (simulando a navegação)
        field->stars[i].position.y += STAR_FIELD_SPEED * deltaTime;

        // 2. Wrap-around (Reciclagem): Se a estrela sair da parte inferior da tela,
        // ela é reposicionada aleatoriamente no topo (y = 0.0f).
        if (field->stars[i].position.y > field->screenHeight) {
            field->stars[i].position = (Vector2){ (float)GetRandomValue(0, field->screenWidth), 0.0f };
        }

        // 3. Controle do Pisca-Pisca (Timer)
        field->stars[i].blinkTimer += deltaTime;
        if (field->stars[i].blinkTimer >= field->stars[i].blinkDuration) {
            field->stars[i].blinkTimer = 0.0f;
            // Inverte o estado: Se estava clareando, passa a escurecer, e vice-versa.
            field->stars[i].isFadingIn = !field->stars[i].isFadingIn;
        }

        // 4. Efeito Fade (Mudança de Alpha/Opacidade)
        if (field->stars[i].isFadingIn) {
            // Clareando: Aumenta o alpha. O Clamp garante que o valor fique entre 0.0f e 1.0f.
            // O multiplicador 2.0f controla a velocidade da transição.
            field->stars[i].currentAlpha = Clamp(field->stars[i].currentAlpha + deltaTime * 2.0f, 0.0f, 1.0f);
        } else {
            // Escurecendo: Diminui o alpha.
            field->stars[i].currentAlpha = Clamp(field->stars[i].currentAlpha - deltaTime * 2.0f, 0.0f, 1.0f);
        }
    }
}

// --- FUNÇÃO DE DESENHO DO CAMPO DE ESTRELAS ---
void DrawStarField(StarField *field) {
    for (int i = 0; i < field->count; i++) {
        // Aplica o alpha atual da estrela (controlado pelo pisca-pisca) à cor WHITE
        Color starColor = Fade(field->stars[i].color, field->stars[i].currentAlpha);

        // Desenha a estrela como um círculo na sua posição e tamanho
        DrawCircleV(field->stars[i].position, field->stars[i].size, starColor);
    }
}

// --- FUNÇÃO DE FINALIZAÇÃO ---
// Libera a memória alocada para o array de estrelas
void UnloadStarField(StarField *field) {
    MemFree(field->stars);
    field->stars = NULL;
    field->count = 0;
}