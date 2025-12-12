#ifndef STAR_H
#define STAR_H // Início do header guard (proteção contra inclusão múltipla)

#include "raylib.h" // Inclui a biblioteca Raylib para tipos como Vector2 e Color
#include <stdbool.h> // Inclui para usar o tipo booleano

// --- CONSTANTES ---

// Velocidade base de rolagem para simular movimento (Paralaxe)
#define STAR_FIELD_SPEED 50.0f
// Limites de tamanho para as estrelas
#define MIN_STAR_SIZE 1.0f
#define MAX_STAR_SIZE 3.0f

// --- ESTRUTURA DE DADOS ---

// Definição da estrela individual
typedef struct {
    Vector2 position;     // Posição (x, y) na tela
    Color color;          // Cor da estrela (inclui o canal alfa)
    float size;           // Tamanho/Raio da estrela

    // Variáveis para o efeito de pulsação/pisca-pisca
    float blinkTimer;     // Temporizador interno para controlar o ciclo de piscar
    float blinkDuration;  // Duração total do ciclo de piscar (Fade In + Fade Out)
    float currentAlpha;   // Transparência atual da estrela
    bool isFadingIn;      // Flag que indica se a estrela está aumentando ou diminuindo o brilho
} Star;

// Definição do Gerenciador de Estrelas (O campo estelar)
typedef struct {
    Star *stars;        // Ponteiro para o array dinâmico de estrelas
    int count;          // Número total de estrelas no campo
    int screenWidth;    // Largura da tela para respawn/gerenciamento de estrelas
    int screenHeight;   // Altura da tela
} StarField;

// --- DECLARAÇÕES DE FUNÇÕES PÚBLICAS ---

// Inicializa o campo estelar, alocando memória para as estrelas
// e definindo suas propriedades iniciais.
void InitStarField(StarField *field, int count, int screenWidth, int screenHeight);

// Atualiza a posição de cada estrela (rolagem) e o estado do temporizador de pulsação.
void UpdateStarField(StarField *field, float deltaTime);

// Desenha todas as estrelas ativas na tela.
void DrawStarField(StarField *field);

// Descarrega o campo estelar, liberando a memória alocada dinamicamente.
void UnloadStarField(StarField *field);

#endif // STAR_H