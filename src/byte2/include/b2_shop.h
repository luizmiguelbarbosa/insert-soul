#ifndef SHOP_H
#define SHOP_H // Início do header guard

#include "raylib.h"     // Tipos básicos: Rectangle, Color, Texture2D, Vector2
#include "b2_player.h"     // Necessário para acessar e modificar o estado do jogador
#include "b2_star.h"       // Necessário para desenhar e atualizar o campo estelar de fundo
#include "b2_game_state.h" // Necessário para transição de estados (e.g., sair da loja)

// --- CONSTANTES E ENUMS DE ITENS ---

// Definições de itens
#define MAX_SHOP_ITEMS 3 // Número máximo de itens disponíveis na loja. (Alterado de 4 para 3, removendo um slot)

// Enumeração dos tipos de itens.
typedef enum {
    ITEM_ENERGY_CHARGE = 0, // Recarrega a energia (ou vida, dependendo do design do jogo)
    ITEM_SHURIKEN,          // Desbloqueia ou aprimora o ataque Shuriken (projéteis laterais)
    ITEM_SHIELD             // Compra um escudo temporário ou um aumento de defesa
    // ITEM_EXTRA_LIFE Removido - Este tipo de item foi descontinuado
} ItemType;

// --- ESTRUTURAS ---

// Estrutura que define um item na loja
typedef struct {
    Rectangle rect;       // Área de colisão/desenho do item na tela
    const char *name;     // Nome visível do item
    int price;            // Custo do item
    Color color;          // Cor de destaque (se necessário para desenho)
    bool active;          // Se o item está disponível para compra
    ItemType type;        // Tipo do item (usado para aplicar o efeito ao jogador)
} ShopItem;

// Estrutura que define o Vendedor (Vendor) animado
typedef struct {
    Texture2D texture;    // Textura da folha de sprite do vendedor
    Rectangle frameRec;   // Retângulo de origem na folha de sprite para o frame atual
    Vector2 position;     // Posição na tela
    float scale;          // Escala de desenho
    int currentFrame;     // Índice do frame atual da animação
    float frameTimer;     // Temporizador para controlar a velocidade da animação
    bool isHappy;         // Flag para a animação de satisfação (após compra)
    float happyTimer;     // Temporizador da animação "feliz"
    int frameCountX;      // Número de frames por linha na folha de sprite
    int frameCountY;      // Número de linhas de frames na folha de sprite
} Vendor;

// Estrutura principal da cena/estado da Loja
typedef struct {
    Vendor vendor;                              // Estrutura do Vendedor
    ShopItem items[MAX_SHOP_ITEMS];             // Array de itens disponíveis
    Texture2D itemTextures[MAX_SHOP_ITEMS];     // Texturas dos ícones dos itens (para desenho)

    Rectangle exitArea;                         // Área de colisão para sair da loja (e.g., um portal)

    float portalParallaxOffset;                 // Offset para o efeito de paralaxe do portal

    char dialogText[256];                       // Texto de diálogo atual do vendedor
    bool itemBought;                            // Flag para indicar que uma compra foi efetuada
    float particleTimer;                        // Temporizador para efeitos de partículas (e.g., após compra)
    bool showParticles;                         // Flag para mostrar partículas
    bool itemFocused;                           // Indica se o jogador está sobrevoando um item (foco)
} ShopScene;

// --- DECLARAÇÕES DE FUNÇÕES PÚBLICAS ---

// Inicializa a cena da loja, carregando texturas, configurando itens e o vendedor.
// AJUSTE: O Player* é necessário para configurar corretamente a loja (e.g., checar upgrades existentes).
void InitShop(ShopScene *shop, Player *player, int gameWidth, int gameHeight);

// Atualiza a lógica da loja: movimento do jogador, verificação de interação (compra/saída)
// e atualização do fundo estelar.
void UpdateShop(ShopScene *shop, Player *player, StarField *stars, GameState *state, float deltaTime);

// Desenha todos os elementos da cena da loja: vendedor, itens, texto, fundo.
void DrawShop(ShopScene *shop, Player *player, StarField *stars);

// Descarrega texturas e libera recursos da loja.
void UnloadShop(ShopScene *shop);

#endif // SHOP_H