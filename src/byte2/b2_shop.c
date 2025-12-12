#include "b2_shop.h"
#include "b2_player.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>

// Definir CLAMP para compatibilidade, se necessário
// Garante que a função Clamp esteja disponível, caso raymath.h não a forneça diretamente.
#ifndef __RAYMATH_H__
    #define CLAMP(v, min, max) ((v) < (min) ? (min) : ((v) > (max) ? (max) : (v)))
    #define Clamp(v, min, max) CLAMP((v), (min), (max))
#endif


// --- CONSTANTES DE APARÊNCIA E GEOMETRIA ---
#define VENDOR_BASE_FRAME_W 64.0f
#define VENDOR_BASE_FRAME_H 64.0f
#define VENDOR_DRAW_SCALE 6.0f // Fator de escala do vendedor (grande na loja)
#define HORIZON_OFFSET_Y -50.0f // Deslocamento do horizonte verticalmente a partir do meio da tela
#define PARTICLE_LIFETIME 0.5f // Duração do efeito de partículas após a compra

// Caminhos das texturas dos itens (Power-ups)
#define ENERGY_POWERUP_PATH "assets/byte2/images/sprites/energy_icon.png"
#define SHURIKEN_PATH "assets/byte2/images/sprites/icone_powerup_shurikens.png"
#define SHIELD_PATH "assets/byte2/images/sprites/shield.png"
// #define EXTRA_LIFE_PATH "assets/images/sprites/vidaextra.png" // REMOVIDO

// Constantes para a caixa de diálogo na parte inferior
#define TEXT_BOX_HEIGHT 100
#define DIALOG_FONT_SIZE 18
#define DIALOG_TEXT_Y_OFFSET 40

// Constantes do Portal de Saída
#define PORTAL_Z_DISTANCE 0.5f
#define PORTAL_BASE_Y_OFFSET 150.0f

#define PORTAL_BRIGHT_CYAN (Color){ 100, 255, 255, 255 }
#define PORTAL_DARK_BLUE (Color){ 0, 0, 100, 255 }

// Tamanho dos ícones dos itens na loja (escalado)
const float ITEM_SIZE_SCALED = 60.0f;


// --- FUNÇÃO DE INICIALIZAÇÃO DA CENA DA LOJA ---
void InitShop(ShopScene *shop, Player *player, int gameWidth, int gameHeight) {
    // Inicializa o Vendedor (Vendor - objeto que representa a loja)
    shop->vendor.frameRec = (Rectangle){ 0.0f, 0.0f, VENDOR_BASE_FRAME_W, VENDOR_BASE_FRAME_H };
    shop->vendor.scale = VENDOR_DRAW_SCALE;
    shop->vendor.isHappy = false;
    shop->vendor.happyTimer = 0.0f;
    shop->portalParallaxOffset = 0.0f; // Offset para o efeito parallax do portal

    // Cálculo da posição da área de saída (Portal)
    float vendorDrawWidth = shop->vendor.frameRec.width * shop->vendor.scale;
    float vendorDrawHeight = shop->vendor.frameRec.height * shop->vendor.scale;
    float horizonY = (float)gameHeight / 2 + HORIZON_OFFSET_Y; // Linha do horizonte

    float collisionY = horizonY + 50.0f;
    float collisionW = 40.0f;
    float collisionH = 20.0f;

    // Define a área de colisão do portal (ponto onde o jogador pode sair)
    shop->exitArea = (Rectangle){
        (float)gameWidth/2 - collisionW / 2,
        collisionY,
        collisionW,
        collisionH
    };

    // Posicionamento inicial do Player na loja (longe da caixa de diálogo)
    float playerH = player->texture.height * player->scale;
    float playerW = player->texture.width * player->scale;
    player->position = (Vector2){
        (float)gameWidth / 2 - playerW / 2,
        600.0f - TEXT_BOX_HEIGHT - playerH - 10.0f // Posicionado acima da caixa de diálogo
    };

    // --- CARREGAMENTO DAS TEXTURAS DOS ITENS ---
    shop->itemTextures[0] = LoadTexture(ENERGY_POWERUP_PATH);
    shop->itemTextures[1] = LoadTexture(SHURIKEN_PATH);
    shop->itemTextures[2] = LoadTexture(SHIELD_PATH);
    // shop->itemTextures[3] = LoadTexture(EXTRA_LIFE_PATH); // REMOVIDO

    // Aplica o filtro POINT para manter a qualidade pixelada
    for (int i = 0; i < MAX_SHOP_ITEMS; i++) {
        if (shop->itemTextures[i].id != 0) SetTextureFilter(shop->itemTextures[i], TEXTURE_FILTER_POINT);
    }


    // --- SETUP DOS ITENS NA LOJA ---
    float midX = (float)gameWidth / 2;
    float floorY = (float)gameHeight / 2 + 60; // Posição vertical dos itens

    float itemSpacing = 20.0f;
    // Calcula a largura total para centralizar os 3 itens
    float totalItemsWidth = (ITEM_SIZE_SCALED * MAX_SHOP_ITEMS) + (itemSpacing * (MAX_SHOP_ITEMS - 1));
    float startX = midX - totalItemsWidth / 2;

    // Inicialização dos dados de cada item (Retângulo de colisão, Nome, Preço, Cor, Ativo, Tipo)
    shop->items[0] = (ShopItem){ { startX, floorY, ITEM_SIZE_SCALED, ITEM_SIZE_SCALED }, "Carga de Energia", 0, WHITE, true, ITEM_ENERGY_CHARGE };
    shop->items[1] = (ShopItem){ { startX + ITEM_SIZE_SCALED + itemSpacing, floorY, ITEM_SIZE_SCALED, ITEM_SIZE_SCALED }, "SHURIKENS", 750, RED, true, ITEM_SHURIKEN };
    shop->items[2] = (ShopItem){ { startX + (ITEM_SIZE_SCALED + itemSpacing) * 2, floorY, ITEM_SIZE_SCALED, ITEM_SIZE_SCALED }, "ESCUDO", 1500, BLUE, true, ITEM_SHIELD };
    // shop->items[3] = (ShopItem){ { startX + (ITEM_SIZE_SCALED + itemSpacing) * 3, floorY, ITEM_SIZE_SCALED, ITEM_SIZE_SCALED }, "VIDA EXTRA", 2250, GREEN, true, ITEM_EXTRA_LIFE }; // REMOVIDO

    shop->particleTimer = 0.0f;
    shop->showParticles = false;

    // Mensagem de diálogo inicial
    sprintf(shop->dialogText, "SEJA BEM-VINDO, VIAJANTE! O UPGRADE DE ENERGIA E POR MINHA CONTA.");
    shop->itemBought = false;
    shop->itemFocused = false;
}

// --- FUNÇÃO PARA DESENHAR O AMBIENTE DA LOJA ---
// Cria um fundo com efeito de grade perspectiva (Grid)
void DrawShopEnvironment(int width, int height) {
    int horizonY = height / 2 + (int)HORIZON_OFFSET_Y;
    int centerX = width / 2;

    // Fundo Gradiente (do chão escuro para o preto)
    DrawRectangleGradientV(0, horizonY, width, height - horizonY, (Color){10, 10, 30, 255}, BLACK);
    // Linha do horizonte (verde neon)
    DrawLine(0, horizonY, width, horizonY, GREEN);

    // Linhas de perspectiva Verticais (convergem para o centro)
    for (int i = -10; i <= 10; i++) {
        Vector2 start = { centerX + (i * 20), horizonY };
        Vector2 end = { centerX + (i * 100), height };
        DrawLineEx(start, end, 1.0f, Fade(GREEN, 0.3f));
    }

    // Linhas de perspectiva Horizontais
    for (int i = 0; i < 10; i++) {
        // Cálculo da posição Y baseado em i*i (curva de perspectiva)
        float y = horizonY + (i * i * 4) + 10;
        if (y > height) break;
        DrawLine(0, (int)y, width, (int)y, Fade(GREEN, 0.3f));
    }

    // Elipse escura no horizonte (simulando sombra do portal ou base do vendedor)
    DrawEllipse(centerX, horizonY - 10, 100, 30, Fade(BLACK, 0.8f));
}

// --- FUNÇÃO DE ATUALIZAÇÃO DA LÓGICA DA LOJA ---
void UpdateShop(ShopScene *shop, Player *player, StarField *stars, GameState *state, float deltaTime) {
    UpdateStarField(stars, deltaTime); // O campo de estrelas continua se movendo

    float pW = player->texture.width * player->scale;
    float pH = player->texture.height * player->scale;
    Rectangle playerRect = { player->position.x, player->position.y, pW, pH };

    // Só permite movimento se o jogador não acabou de comprar um item (transição de compra)
    if (!shop->itemBought) {
        float speed = player->speed * deltaTime;
        // Movimento do jogador
        if (IsKeyDown(KEY_LEFT)) player->position.x -= speed;
        if (IsKeyDown(KEY_RIGHT)) player->position.x += speed;
        if (IsKeyDown(KEY_UP)) player->position.y -= speed;
        if (IsKeyDown(KEY_DOWN)) player->position.y += speed;

        // Limita o movimento horizontal do jogador
        player->position.x = Clamp(player->position.x, 0.0f, 800.0f - pW);

        // Limita o movimento vertical (não pode passar da área do portal para cima nem da caixa de diálogo para baixo)
        player->position.y = Clamp(player->position.y, shop->exitArea.y - pH, 600.0f - pH - TEXT_BOX_HEIGHT);

        // --- Efeito Parallax do Portal ---
        // Calcula a posição relativa do jogador no eixo X (de -1.0 a 1.0)
        float playerRelativeX = (player->position.x - (800.0f / 2.0f)) / (800.0f / 2.0f);
        // Aplica o offset para mover o portal ligeiramente na direção oposta ao jogador
        shop->portalParallaxOffset = playerRelativeX * 50.0f * PORTAL_Z_DISTANCE;

        // --- LÓGICA DE SAÍDA (PORTAL) ---
        if (CheckCollisionRecs(playerRect, shop->exitArea)) {
            // Se o jogador está na área de saída (portal)
            // Informa que precisa de 'P' para sair
            sprintf(shop->dialogText, "PORTAL PRONTO! Pressione P para sair e voltar a acao.");

            if (IsKeyPressed(KEY_P)) {
                // Transição para o estado de Gameplay
                *state = STATE_GAMEPLAY;
                // Reposiciona o jogador no mapa de gameplay
                player->position = (Vector2){ 400 - pW/2, 600 - 100 };
                return;
            }
        }
    }

    // --- Lógica do Timer do Vendedor ---
    if (shop->vendor.isHappy) {
        shop->vendor.happyTimer += deltaTime;
        if (shop->vendor.happyTimer > 3.0f) {
            shop->vendor.isHappy = false;
            shop->vendor.happyTimer = 0.0f;
        }
    }

    // --- Lógica de Transição de Compra (Partículas) ---
    if (shop->itemBought) {
        shop->showParticles = true;
        shop->particleTimer += deltaTime;

        if (shop->particleTimer > PARTICLE_LIFETIME) {
            // Fim da transição
            shop->itemBought = false;
            shop->particleTimer = 0.0f;
            shop->showParticles = false;
            shop->vendor.isHappy = true; // Vendedor fica feliz brevemente após a compra
        }
    }

    // --- LÓGICA DE INTERAÇÃO COM ITENS (Fora da Transição) ---
    if (!shop->itemBought) {
        bool isPlayerNearItem = false;

        // Loop sobre os itens da loja
        for (int i = 0; i < MAX_SHOP_ITEMS; i++) {
            // Condição para checar: O item está ativo OU é o upgrade de energia e o jogador ainda não o tem.
            bool shouldCheck = shop->items[i].active || (shop->items[i].type == ITEM_ENERGY_CHARGE && !player->canCharge);

            if (shouldCheck) {
                if (CheckCollisionRecs(playerRect, shop->items[i].rect)) {
                    isPlayerNearItem = true;

                    // --- ITEM: CARGA DE ENERGIA (GRÁTIS, ÚNICO) ---
                    if (shop->items[i].type == ITEM_ENERGY_CHARGE) {
                        if (!player->canCharge) {
                            sprintf(shop->dialogText, "UPGRADE DE ENERGIA: HABILITA O TIRO CARREGADO! PRESSIONE E.");

                            if (IsKeyPressed(KEY_E)) {
                                player->canCharge = true;
                                shop->itemBought = true;
                                shop->items[i].active = false; // Desativa o item na loja
                                sprintf(shop->dialogText, "SISTEMAS ONLINE! CARGA DE ENERGIA HABILITADA.");
                            }
                        }
                    }
                    // --- ITENS PAGOS ---
                    else if (shop->items[i].active) {
                        char priceText[32];
                        sprintf(priceText, "$%d", shop->items[i].price);

                        sprintf(shop->dialogText, "COMPRAR %s POR %s? PRESSIONE E.", shop->items[i].name, priceText);

                        if (IsKeyPressed(KEY_E)) {
                            // Verifica se tem ouro suficiente
                            if (player->gold >= shop->items[i].price) {

                                // Aplica o upgrade
                                switch (shop->items[i].type) {
                                    case ITEM_SHURIKEN:
                                        player->hasDoubleShot = true;
                                        // Se a textura de shuriken estiver carregada, troca a textura do jogador
                                        if (player->shurikenTexture.id != 0) {
                                            player->texture = player->shurikenTexture;
                                        }
                                        break;
                                    case ITEM_SHIELD:
                                        player->hasShield = true;
                                        player->maxLives = 5; // Aumenta a vida máxima
                                        player->currentLives = 5; // Cura o jogador ao receber o upgrade
                                        if (player->shieldTextureAppearance.id != 0) {
                                            player->texture = player->shieldTextureAppearance;
                                        }
                                        break;
                                    default: break;
                                }

                                // Finaliza a compra
                                player->gold -= shop->items[i].price;
                                shop->itemBought = true;
                                shop->items[i].active = false;
                                sprintf(shop->dialogText, "NEGOCIO FECHADO! %s ATIVADO! Use o PORTAL para sair.", shop->items[i].name);
                            } else {
                                // Mensagem de erro por falta de ouro
                                sprintf(shop->dialogText, "CREDITOS INSUFICIENTES! Voce precisa de mais $%d para comprar %s.", shop->items[i].price, shop->items[i].name);
                            }
                        }
                    }
                }
            }
        }

        // --- MENSAGEM PADRÃO ---
        // Se o jogador não está perto de um item OU do portal, mostra a mensagem padrão.
        if (!isPlayerNearItem && !CheckCollisionRecs(playerRect, shop->exitArea)) {
            if (!player->canCharge) {
                 sprintf(shop->dialogText, "SEJA BEM-VINDO, VIAJANTE! O UPGRADE DE ENERGIA E POR MINHA CONTA.");
            } else {
                sprintf(shop->dialogText, "EXPLORE OS PRODUTOS! Entre no portal e aperte P para SAIR. CREDITOS: $%d", player->gold);
            }
        }
    }
}

// --- FUNÇÃO DE DESENHO DA CENA DA LOJA ---
void DrawShop(ShopScene *shop, Player *player, StarField *stars) {
    DrawStarField(stars); // Desenha o fundo de estrelas
    DrawShopEnvironment(800, 600); // Desenha a grade de perspectiva

    // Tamanho do vendedor para calcular o portal
    float vendorDrawWidth = VENDOR_BASE_FRAME_W * VENDOR_DRAW_SCALE;
    float vendorDrawHeight = VENDOR_BASE_FRAME_H * VENDOR_DRAW_SCALE;

    // --- DESENHO DO PORTAL DE SAÍDA ---
    float portalBaseY = shop->exitArea.y + shop->exitArea.height / 2;
    float time = GetTime();
    float pulse = sin(time * 4.0f) * 0.1f + 0.9f; // Pulsação para efeito animado

    // O retângulo visual do portal é ajustado pelo parallax
    Rectangle portalVisualRect = {
        shop->exitArea.x + shop->portalParallaxOffset - (vendorDrawWidth - shop->exitArea.width) / 2,
        portalBaseY - vendorDrawHeight,
        vendorDrawWidth,
        vendorDrawHeight
    };
    float portalDrawCenterX = portalVisualRect.x + portalVisualRect.width / 2;

    // Cores do gradiente com pulsação
    Color topColor = Fade(SKYBLUE, 0.7f * pulse);
    Color bottomColor = Fade(BLUE, 0.9f * pulse);

    // Efeito de glow na base do portal
    DrawCircleGradient(
        (int)portalDrawCenterX,
        (int)(portalVisualRect.y + portalVisualRect.height),
        portalVisualRect.width / 2.0f * pulse,
        PORTAL_BRIGHT_CYAN,
        Fade(PORTAL_DARK_BLUE, 0.0f)
    );

    // Corpo principal do portal (gradiente vertical)
    DrawRectangleGradientV(
        (int)portalDrawCenterX - (int)portalVisualRect.width / 2,
        (int)portalVisualRect.y,
        (int)portalVisualRect.width,
        (int)portalVisualRect.height,
        topColor,
        bottomColor
    );

    // Contorno do portal
    DrawRectangleLinesEx(portalVisualRect, 3.0f, Fade((Color){ 0, 255, 255, 255 }, 0.8f * pulse));

    // Partículas/estrelas ao redor do portal (efeito animado)
    for (int i = 0; i < 5; i++) {
        // Cálculo de posição para as partículas orbitarem o portal
        float angle = time * (10 + i * 2) + i * 0.5f;
        float radius = portalVisualRect.width / 2.0f + sin(time * (3 + i)) * 10.0f;
        float xOffset = cos(angle) * radius;
        float yOffset = sin(angle) * radius * 0.5f;

        Color brightPulseColor = Fade(PORTAL_BRIGHT_CYAN, 0.5f + sin(time * (5 + i)) * 0.3f);
        DrawCircleV((Vector2){ portalDrawCenterX + xOffset, portalVisualRect.y + portalVisualRect.height/2 + yOffset }, 5.0f * pulse, brightPulseColor);
    }

    // Texto do portal
    DrawText("PORTAL DE SAIDA",
        (int)portalDrawCenterX - MeasureText("PORTAL DE SAIDA", 20)/2,
        (int)(portalVisualRect.y + portalVisualRect.height) - 30,
        20,
        Fade(WHITE, 0.9f)
    );
    // --- FIM DO DESENHO DO PORTAL ---


    // --- DESENHO DOS ITENS ---
    for (int i = 0; i < MAX_SHOP_ITEMS; i++) {
        // Desenha o item se estiver ativo OU se for o upgrade de energia e o jogador ainda não o tiver
        bool shouldDrawItem = shop->items[i].active || (shop->items[i].type == ITEM_ENERGY_CHARGE && !player->canCharge);

        if (shouldDrawItem) {
            Texture2D itemTexture = shop->itemTextures[i];
            float floatY = sin(time * 3 + i) * 3; // Efeito de flutuação vertical
            Rectangle drawRect = shop->items[i].rect;
            drawRect.y += floatY;

            // Desenho da caixa do item (com efeito de glow/sombra)
            DrawRectangleRec(drawRect, Fade(shop->items[i].color, 0.4f));
            DrawRectangleLinesEx(drawRect, 2, WHITE);

            // Desenho da textura do item
            if (itemTexture.id != 0) {
                float textureScale = drawRect.width / itemTexture.width;
                Vector2 pos = { drawRect.x, drawRect.y };
                DrawTextureEx(itemTexture, pos, 0.0f, textureScale, WHITE);
            }

            // Nome do Item
            int nameWidth = MeasureText(shop->items[i].name, 10);
            DrawText(shop->items[i].name, (int)drawRect.x + (int)drawRect.width/2 - nameWidth/2, (int)drawRect.y - 25, 10, WHITE);

            // Preço do Item
            char priceText[16];
            if (shop->items[i].price == 0) sprintf(priceText, "GRATIS");
            else sprintf(priceText, "$%d", shop->items[i].price);

            int priceWidth = MeasureText(priceText, 10);
            DrawText(priceText, (int)drawRect.x + (int)drawRect.width/2 - priceWidth/2, (int)drawRect.y + (int)drawRect.height + 5, 10, YELLOW);
        }
    }

    // --- EFEITO DE PARTÍCULAS PÓS-COMPRA ---
    if (shop->showParticles) {
        float pW = player->texture.width * player->scale;
        float pH = player->texture.height * player->scale;
        Vector2 playerCenter = { player->position.x + pW/2, player->position.y + pH/2 };

        Color effectColor = LIME;

        // O raio do círculo de partículas se expande ao longo do tempo de vida (PARTICLE_LIFETIME)
        float radius = (shop->particleTimer / PARTICLE_LIFETIME) * 60.0f;
        // O alpha da cor diminui ao longo do tempo (fade-out)
        Color particleColor = Fade(effectColor, 1.0f - (shop->particleTimer / PARTICLE_LIFETIME));
        DrawCircleLines((int)playerCenter.x, (int)playerCenter.y, radius, particleColor);
    }

    DrawPlayer(player); // Desenha o jogador

    // --- HUD e CAIXA DE DIÁLOGO ---
    // Exibe os créditos/ouro do jogador
    DrawText(TextFormat("CREDITOS: $%d", player->gold), 10, 10, 20, GOLD);

    // Desenha o background da caixa de diálogo
    int boxH = TEXT_BOX_HEIGHT;
    DrawRectangle(0, 600 - boxH, 800, boxH, Fade(BLACK, 0.9f));
    // Contorno da caixa de diálogo
    DrawRectangleLines(0, 600 - boxH, 800, boxH, GREEN);

    // Desenha o texto de diálogo (instruções, preços, mensagens)
    DrawTextEx(GetFontDefault(), shop->dialogText, (Vector2){ 20, 600 - DIALOG_TEXT_Y_OFFSET }, DIALOG_FONT_SIZE, 1, GREEN);
}

// --- FUNÇÃO DE FINALIZAÇÃO ---
// Descarrega todas as texturas de item carregadas
void UnloadShop(ShopScene *shop) {
    for (int i = 0; i < MAX_SHOP_ITEMS; i++) {
        if (shop->itemTextures[i].id != 0) UnloadTexture(shop->itemTextures[i]);
    }
}