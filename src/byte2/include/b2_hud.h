#ifndef HUD_H
#define HUD_H

#include "raylib.h"
#include <stdbool.h>

// --- ESTRUTURA DE DADOS ---

/**
 * @brief Estrutura que armazena as texturas e o estado básico do HUD.
 */
typedef struct {
    int score; // Pontuação atual do jogo.

    // Ícones de Status
    Texture2D lifeIconTexture;      // Ícone usado para representar a vida.
    Texture2D energyIconTexture;    // Ícone usado para a barra de carga/energia.

    // Ícones de Upgrades
    Texture2D shurikenTexture;      // Ícone para o upgrade Shuriken.
    Texture2D shieldTexture;        // Ícone para o upgrade Escudo.

    // Ícone de Moeda
    Texture2D goldTexture;          // Ícone para a moeda (ouro).

} Hud;

// --- DECLARAÇÕES DE FUNÇÕES PÚBLICAS ---

/**
 * @brief Inicializa o HUD, carregando todas as texturas necessárias.
 */
void InitHud(Hud *hud);

/**
 * @brief Atualiza a lógica interna do HUD (ex: animações, contadores).
 */
void UpdateHud(Hud *hud, float deltaTime);

/**
 * @brief Desenha a barra lateral do HUD com todos os indicadores de status e upgrades.
 * * @param isLeft Se o HUD é desenhado na esquerda (true) ou direita (false).
 * @param marginHeight Altura da área de desenho do HUD.
 * @param energyCharge Nível de carga de energia do jogador (para a barra de progresso).
 * @param hasDoubleShot Estado do upgrade Shuriken.
 * @param hasShield Estado do upgrade Escudo.
 * @param extraLives Número de vidas extras.
 * @param drawLives Vida atual (para desenhar os corações/ícones de vida).
 * @param currentGold Quantidade de ouro atual.
 */
void DrawHudSide(Hud *hud, bool isLeft, int marginHeight, float energyCharge, bool hasDoubleShot, bool hasShield, int extraLives, int drawLives, int currentGold);


/**
 * @brief Descarrega todas as texturas do HUD.
 */
void UnloadHud(Hud *hud);

#endif // HUD_H