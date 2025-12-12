#ifndef ENEMY_H
#define ENEMY_H // Início do header guard

#include "raylib.h"
#include "b2_audio.h" // Necessário para acionar SFX de explosão.

// --- CONSTANTES DA FORMAÇÃO DE INIMIGOS NORMAIS ---
#define ENEMY_COLS 11                       // Número de colunas na formação.
#define ENEMY_ROWS 5                        // Número de linhas na formação.
#define ENEMY_COUNT (ENEMY_COLS * ENEMY_ROWS) // Total de inimigos.
#define ENEMY_SIZE 40.0f                    // Tamanho base de cada inimigo.
#define ENEMY_PADDING_X 15.0f               // Espaçamento horizontal entre inimigos.
#define ENEMY_PADDING_Y 15.0f               // Espaçamento vertical entre inimigos.
#define ENEMY_SPEED_INITIAL 100.0f          // Velocidade inicial de movimento horizontal.
#define ENEMY_DROP_AMOUNT 20.0f             // Quantidade que os inimigos caem ao atingir a borda.
#define ENEMY_FLASH_DURATION 0.1f           // Duração do flash visual ao serem atingidos.
#define ENEMY_EXPLOSION_DURATION 0.4f       // Duração da animação de explosão.
#define ENEMY_GAME_OVER_Y 550.0f            // Posição Y que, se atingida, causa Game Over.
#define ENEMY_GAME_OVER_LINE_Y 550.0f       // Linha de alerta visual (restaurada/reafirmada).

// --- CONSTANTES DO SISTEMA DE PARTÍCULAS ---
#define MAX_PARTICLES 500                   // Número máximo de partículas de explosão ativas.
#define PARTICLE_LIFESPAN 0.8f              // Duração da vida de uma partícula em segundos.

// --- CONSTANTES DO BOSS ---
#define BOSS_FRAME_COUNT 30                 // Número de frames de animação do Boss.
#define BOSS_ANIMATION_SPEED 0.08f          // Tempo de exibição de cada frame.
#define BOSS_INITIAL_HEALTH 5500            // Vida inicial do Boss.
#define BOSS_SIZE_WIDTH 128.0f              // Largura do Boss.
#define BOSS_SIZE_HEIGHT 128.0f             // Altura do Boss.

// --- ESTRUTURAS DE DADOS ---

/**
 * @brief Representa uma única partícula de explosão.
 */
typedef struct {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float life;         // Tempo de vida restante.
    bool active;
} Particle;

/**
 * @brief Gerenciador do pool de partículas.
 */
typedef struct {
    Particle particles[MAX_PARTICLES];
    int nextParticleIndex; // Índice para o próximo slot livre.
} ParticleManager;

/**
 * @brief Estrutura para um inimigo normal (do tipo grid/formação).
 */
typedef struct {
    Vector2 position;
    Rectangle rect;
    int type;           // Tipo de inimigo (pode influenciar vida/textura).
    int health;
    bool active;
    float hitTimer;     // Temporizador para o flash de acerto.
    Color neonColor;    // Cor de néon para o visual.
    bool isExploding;   // Flag para a animação de explosão.
    float explosionTimer; // Temporizador da animação de explosão.
} Enemy;

/**
 * @brief Estrutura para o inimigo Boss.
 */
typedef struct Boss {
    bool active;
    Vector2 position;
    int health;
    int maxHealth;
    float hitTimer;
    Rectangle rect;

    // Animação
    int currentFrame;
    float frameTimer;

    // Movimento
    float movementTimer;    // Temporizador para o padrão de movimento.
    Vector2 targetPosition; // Posição alvo para onde o Boss está se movendo.

} Boss;

/**
 * @brief Gerenciador principal de todos os inimigos e partículas.
 */
typedef struct EnemyManager {
    Enemy enemies[ENEMY_COUNT];     // Array da formação de inimigos normais.
    Texture2D enemyTextures[3];     // Texturas para diferentes tipos de inimigos.
    float speed;                    // Velocidade horizontal atual.
    int direction;                  // Direção de movimento (+1 direita, -1 esquerda).
    int activeCount;                // Número de inimigos normais ativos.
    bool gameOver;                  // Flag para indicar Game Over pela invasão.

    // Gerenciamento de Ondas
    int currentWave;
    float waveStartTimer;
    int gameHeight;                 // Altura da tela (para limites).

    int wavesCompletedCount;        // Contador de ondas completadas.
    bool triggerShopReturn;         // Flag para indicar que o jogador deve retornar à loja.

    ParticleManager particleManager; // Gerenciador de Partículas.

    // Gerenciamento do Boss
    Boss boss;
    Texture2D bossFrames[BOSS_FRAME_COUNT]; // Texturas de animação do Boss.
    bool bossActive;                        // Flag: O Boss está ativo? (Substituindo a formação normal).
} EnemyManager;

// Declarações forward para dependências
typedef struct BulletManager BulletManager;
typedef struct AudioManager AudioManager;

// --- DECLARAÇÕES DE FUNÇÕES PÚBLICAS ---

/**
 * @brief Inicializa o gerenciador de inimigos, carregando assets e a primeira formação/onda.
 */
void InitEnemyManager(EnemyManager *manager, int screenWidth, int screenHeight);

/**
 * @brief Atualiza a lógica de todos os inimigos: movimento, timers, explosões, e verifica invasão.
 */
void UpdateEnemies(EnemyManager *manager, float deltaTime, int screenWidth, int *playerLives, bool *gameOver);

/**
 * @brief Desenha todos os inimigos ativos, o Boss, e as partículas de explosão.
 */
void DrawEnemies(EnemyManager *manager);

/**
 * @brief Descarrega todas as texturas de inimigos e Boss.
 */
void UnloadEnemyManager(EnemyManager *manager);

/**
 * @brief Verifica se a onda atual (ou o Boss) foi derrotada e prepara a próxima.
 */
void CheckWaveCompletion(EnemyManager *manager, int screenWidth, int screenHeight);

/**
 * @brief Verifica colisões entre os projéteis do jogador e os inimigos/Boss, aplicando dano.
 */
void CheckBulletEnemyCollision(BulletManager *bulletManager, EnemyManager *enemyManager, int *playerGold, AudioManager *audioManager);

#endif // ENEMY_H