#ifndef PLAYER_H
#define PLAYER_H // Início do header guard

#include "raylib.h" // Tipos básicos: Vector2, Texture2D
#include "b2_bullet.h" // Necessário para interagir com o sistema de projéteis
#include "b2_hud.h"    // Necessário para interagir com a interface do usuário (ex: barra de energia)
#include "b2_audio.h"
// A struct AudioManager é declarada aqui, mas definida em outro arquivo.
// Isso permite usar o tipo 'AudioManager*' nas funções sem incluir o arquivo 'audio.h' inteiro.
typedef struct AudioManager AudioManager;

// --- CONSTANTES ---

#define PLAYER_SCALE 0.2f // Fator de escala da textura da nave do jogador
#define ATTACK_WEAK 1     // Tipo de ataque 1: Fraco
#define ATTACK_MEDIUM 2   // Tipo de ataque 2: Médio
#define ATTACK_STRONG 3   // Tipo de ataque 3: Forte (Carregado ao máximo)

// --- ESTRUTURA DO JOGADOR ---

typedef struct {
    // --- Gráficos e Posição ---
    Texture2D texture;                  // Textura atual da nave (pode mudar se estiver com escudo/aura)
    Texture2D baseTexture;              // Textura base da nave
    Texture2D shurikenTexture;          // Textura de um possível upgrade visual (não usado diretamente no desenho da nave, mas talvez como ícone)
    Texture2D shieldTextureAppearance;  // Textura para o escudo (visual)
    Texture2D extraLifeTextureAppearance; // Textura para vida extra (visual)
    Vector2 position;                   // Posição (x, y) na tela (geralmente o centro da nave)
    float speed;                        // Velocidade de movimento
    float scale;                        // Escala de desenho (deve ser igual a PLAYER_SCALE)

    // --- Economia e Recursos ---
    int gold;                           // Moeda do jogo (coletada/gasta na loja)

    // --- Campos de Vida Adicionados (Gerenciamento de Vidas/HP) ---
    int maxLives;                       // Número máximo de vidas ou pontos de vida (HP)
    int currentLives;                   // Número atual de vidas ou pontos de vida (HP)
    // -----------------------------------------------------------------

    // --- Sistema de Carga de Ataque ---
    float energyCharge;                 // Valor atual do carregamento (0.0 a 1.0 ou mais)
    bool isCharging;                    // Flag: O jogador está pressionando o botão para carregar?
    bool canCharge;                     // Flag: O jogador pode carregar neste momento?

    // --- Upgrades e Estado ---
    bool hasDoubleShot;                 // Upgrade: Se a nave possui o disparo duplo (shuriken)
    bool hasShield;                     // Upgrade: Se a nave possui um escudo ativo
    int extraLives;                     // Upgrade: Vidas extras (se o sistema for baseado em vidas)

    // --- Efeitos Visuais (Aura de Carga/Dano) ---
    float auraRadius;                   // Raio da aura de brilho (usada para indicar carga ou dano)
    float auraAlpha;                    // Transparência (alpha) da aura
    float auraPulseSpeed;               // Velocidade de pulsação da aura

} Player;

// --- DECLARAÇÕES DE FUNÇÕES PÚBLICAS ---

// Inicializa a nave do jogador, carregando texturas e resetando estatísticas.
void InitPlayer(Player *player);

// Atualiza a lógica do jogador: movimento, carregamento de ataque, disparo, checagem de HUD.
// Requer o Player, Gerenciador de Balas, Gerenciador de Áudio, HUD, delta time e dimensões da tela.
void UpdatePlayer(Player *player, BulletManager *bulletManager, AudioManager *audioManager, Hud *hud, float deltaTime, int screenWidth, int screenHeight);

// Desenha a nave do jogador e quaisquer efeitos visuais (aura/escudo).
void DrawPlayer(Player *player);

// Descarrega as texturas e libera recursos do jogador.
void UnloadPlayer(Player *player);

// Calcula o tipo de ataque (WEAK, MEDIUM, STRONG) baseado no valor de carregamento atual.
int CalculateAttackType(float charge);

#endif // PLAYER_H