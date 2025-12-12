#ifndef BULLET_H
#define BULLET_H

#include "raylib.h" // Inclui tipos de dados como Rectangle, Vector2 e Texture2D
#include <stdbool.h>

// --- CONSTANTES ---

#define MAX_PLAYER_BULLETS 20 // Número máximo de projéteis que o jogador pode ter ativos simultaneamente (pool).

// Tipos de ataque (usados para determinar a textura, dano e comportamento)
#define ATTACK_WEAK 1
#define ATTACK_MEDIUM 2
#define ATTACK_STRONG 3
#define ATTACK_SHURIKEN 4 // O ataque lateral (projétil especial)

// --- ESTRUTURAS DE DADOS ---

/**
 * @brief Estrutura que representa um único projétil disparado pelo jogador.
 */
typedef struct {
    Rectangle rect;     // Posição e dimensões (usado para colisão e desenho).
    Vector2 speed;      // Vetor de velocidade (dx/dt, dy/dt).
    bool active;        // Flag para indicar se a bala está em uso ou no pool.
    Color color;        // Cor da bala (pode ser usado para brilho).
    int type;           // Tipo de ataque que gerou a bala (para referência de dano/textura).
} Bullet;

/**
 * @brief Gerenciador do pool de projéteis do jogador.
 */
typedef struct BulletManager {
    Bullet bullets[MAX_PLAYER_BULLETS]; // Array de projéteis (o pool).

    // Texturas dos diferentes tipos de ataque.
    Texture2D weakTexture;
    Texture2D mediumTexture;
    Texture2D strongTexture;
    Texture2D shurikenTexture;
} BulletManager;

// --- DECLARAÇÕES DE FUNÇÕES PÚBLICAS ---

/**
 * @brief Inicializa o gerenciador de projéteis, carregando as texturas.
 */
void InitBulletManager(BulletManager *manager);

/**
 * @brief Dispara o ataque principal do jogador (fraco, médio ou forte), e opcionalmente os shurikens.
 * @param playerCenter Posição central da nave do jogador.
 * @param playerHeight Altura da nave (para posicionar o projétil acima).
 * @param attackType O tipo de ataque carregado (ATTACK_WEAK, etc.).
 * @param hasShurikens Se o upgrade de Shuriken está ativo.
 */
void ShootChargedAttack(BulletManager *manager, Vector2 playerCenter, float playerHeight, int attackType, bool hasShurikens);

/**
 * @brief Atualiza a posição de todos os projéteis ativos e verifica se saíram da tela.
 */
void UpdatePlayerBullets(BulletManager *manager, float deltaTime);

/**
 * @brief Desenha todos os projéteis ativos na tela.
 */
void DrawPlayerBullets(BulletManager *manager);

/**
 * @brief Descarrega as texturas dos projéteis.
 */
void UnloadBulletManager(BulletManager *manager);

#endif // BULLET_H