#include "b2_hud.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>

// --- CAMINHOS DAS TEXTURAS/ÍCONES ---
#define LIFE_ICON_PATH "assets/byte2/images/sprites/life_icon_large.png"
#define ENERGY_ICON_PATH "assets/byte2/images/sprites/energy_icon.png"

#define SHURIKEN_PATH "assets/byte2/images/sprites/icone_powerup_shurikens.png"
#define SHIELD_PATH "assets/byte2/images/sprites/shield.png"

// Caminho para a textura do Gold (Ouro/Créditos)
#define GOLD_PATH "assets/byte2/images/sprites/gold.png"

#define ICON_HUD_SCALE 0.12f // Fator de escala para os ícones no HUD lateral

// --- FUNÇÃO DE INICIALIZAÇÃO DO HUD ---
void InitHud(Hud *hud) {
    hud->score = 0;
    // O campo 'hud->lives' foi removido e a vida agora é gerenciada pelo Player.

    // Carregamento da textura do ícone de Vidas
    hud->lifeIconTexture = LoadTexture(LIFE_ICON_PATH);
    if (hud->lifeIconTexture.id != 0) SetTextureFilter(hud->lifeIconTexture, TEXTURE_FILTER_POINT);
    else printf("[ERRO] Icone de Vida nao encontrado: %s\n", LIFE_ICON_PATH);

    // Carregamento da textura do ícone de Energia
    hud->energyIconTexture = LoadTexture(ENERGY_ICON_PATH);
    if (hud->energyIconTexture.id != 0) SetTextureFilter(hud->energyIconTexture, TEXTURE_FILTER_POINT);
    else printf("[ERRO] Icone de Energia nao encontrado: %s\n", ENERGY_ICON_PATH);

    // Carregamento da textura do ícone de Shurikens (Double Shot)
    hud->shurikenTexture = LoadTexture(SHURIKEN_PATH);
    if (hud->shurikenTexture.id != 0) SetTextureFilter(hud->shurikenTexture, TEXTURE_FILTER_POINT);
    else printf("[ERRO] Icone de Shurikens nao encontrado: %s\n", SHURIKEN_PATH);

    // Carregamento da textura do ícone de Escudo (Shield)
    hud->shieldTexture = LoadTexture(SHIELD_PATH);
    if (hud->shieldTexture.id != 0) SetTextureFilter(hud->shieldTexture, TEXTURE_FILTER_POINT);
    else printf("[ERRO] Icone de Escudo nao encontrado: %s\n", SHIELD_PATH);

    // Carregamento da Textura do Gold (Ouro)
    hud->goldTexture = LoadTexture(GOLD_PATH);
    if (hud->goldTexture.id != 0) SetTextureFilter(hud->goldTexture, TEXTURE_FILTER_POINT);
    else printf("[ERRO] Icone de Gold nao encontrado: %s\n", GOLD_PATH);
}

// --- FUNÇÃO DE ATUALIZAÇÃO DO HUD ---
// Atualmente vazia, pois o HUD é principalmente visual e usa dados do Player no momento do desenho.
void UpdateHud(Hud *hud, float deltaTime) {
    // Não há lógica de atualização de estado aqui (score e lives são atualizados por outros módulos).
}

// --- FUNÇÃO PARA DESENHAR O HUD LATERAL ---
// Esta função é chamada duas vezes no byte2.c: uma para o lado esquerdo (informações do jogador)
// e outra para o lado direito (geralmente apenas o ouro).
void DrawHudSide(Hud *hud, bool isLeft, int marginHeight, float energyCharge, bool hasDoubleShot, bool hasShield, int extraLives, int drawLives, int currentGold) {
    int fontSize = 20;
    int screenW = GetScreenWidth(); // Largura real da tela (monitor)
    int targetY = 10; // Posição Y inicial

    // Define as dimensões do ícone com base na textura de vida (assumindo que todas as texturas têm a mesma base)
    float iconTextureWidth = 64.0f;
    float iconTextureHeight = 64.0f;
    if (hud->lifeIconTexture.id != 0) iconTextureWidth = (float)hud->lifeIconTexture.width;
    if (hud->lifeIconTexture.id != 0) iconTextureHeight = (float)hud->lifeIconTexture.height;

    // Calcula o tamanho final de desenho do ícone (escalado)
    float iconDrawWidth = iconTextureWidth * ICON_HUD_SCALE;
    float iconDrawHeight = iconTextureHeight * ICON_HUD_SCALE;

    // --- LADO ESQUERDO (VIDAS, ENERGIA, POWER-UPS) ---
    if (isLeft) {
        int iconX = 10;
        float currentY = (float)targetY;

        // 1. Vidas
        if (hud->lifeIconTexture.id != 0) {
            // Desenha o ícone de vida
            Rectangle sourceRecLife = { 0.0f, 0.0f, iconTextureWidth, iconTextureHeight };
            Rectangle destRecLife = { (float)iconX, currentY, iconDrawWidth, iconDrawHeight };
            DrawTexturePro(hud->lifeIconTexture, sourceRecLife, destRecLife, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
        }

        // Desenha o contador de vidas (base + extras)
        char livesText[10];
        sprintf(livesText, "x%02d", drawLives + extraLives);

        int textX = iconX + (int)iconDrawWidth + 5;
        int textY = (int)currentY + (int)iconDrawHeight / 2 - fontSize / 2;
        DrawText(livesText, textX, textY, fontSize, WHITE);
        currentY += iconDrawHeight + 15.0f; // Move o Y para o próximo item

        // 2. Carga de Energia
        if (hud->energyIconTexture.id != 0) {
            Rectangle energySourceRec = { 0.0f, 0.0f, (float)hud->energyIconTexture.width, (float)hud->energyIconTexture.height };
            Rectangle energyDestRec = { (float)iconX, currentY, iconDrawWidth, iconDrawHeight };

            // O ícone fica levemente transparente se não houver carga (0.0%)
            Color energyColor = (energyCharge > 0.0f) ? WHITE : Fade(WHITE, 0.5f);
            DrawTexturePro(hud->energyIconTexture, energySourceRec, energyDestRec, (Vector2){ 0.0f, 0.0f }, 0.0f, energyColor);

            // Desenha o percentual de carga
            char chargeText[10];
            sprintf(chargeText, "%d%%", (int)round(energyCharge));

            int textEnergyX = iconX + (int)iconDrawWidth + 5;
            int textEnergyY = (int)currentY + (int)iconDrawHeight / 2 - fontSize / 2;
            DrawText(chargeText, textEnergyX, textEnergyY, fontSize, WHITE);
            currentY += iconDrawHeight + 15.0f;
        }

        // 3. Power-ups Ativos
        // Ícone de Shurikens (Double Shot)
        if (hasDoubleShot) {
            if (hud->shurikenTexture.id != 0) {
                Rectangle source = { 0.0f, 0.0f, (float)hud->shurikenTexture.width, (float)hud->shurikenTexture.height };
                Rectangle dest = { (float)iconX, currentY, iconDrawWidth, iconDrawHeight };
                DrawTexturePro(hud->shurikenTexture, source, dest, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                currentY += iconDrawHeight + 10.0f;
            }
        }

        // Ícone de Escudo (Shield)
        if (hasShield) {
            if (hud->shieldTexture.id != 0) {
                Rectangle source = { 0.0f, 0.0f, (float)hud->shieldTexture.width, (float)hud->shieldTexture.height };
                Rectangle dest = { (float)iconX, currentY, iconDrawWidth, iconDrawHeight };
                DrawTexturePro(hud->shieldTexture, source, dest, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                currentY += iconDrawHeight + 10.0f;
            }
        }

    }
    // --- LADO DIREITO (GOLD/OURO) ---
    else {
        float currentY = (float)targetY;

        // O score foi removido e substituído por Gold.

        // 1. Gold (Ouro/Créditos)
        if (hud->goldTexture.id != 0) {
            // Prepara o texto do Gold (Ouro)
            char goldText[32];
            sprintf(goldText, "%d", currentGold);
            int goldTextWidth = MeasureText(goldText, fontSize);

            // Calcula a posição do texto (alinha à direita: largura total - largura do texto - margem)
            int textGoldX = screenW - goldTextWidth - 10;
            int textGoldY = (int)currentY + (int)iconDrawHeight / 2 - fontSize / 2;

            // Calcula a posição do ícone (ícone à esquerda do texto)
            int iconX = textGoldX - (int)iconDrawWidth - 5;

            // Desenha o ícone
            Rectangle sourceRecGold = { 0.0f, 0.0f, (float)hud->goldTexture.width, (float)hud->goldTexture.height };
            Rectangle destRecGold = { (float)iconX, currentY, iconDrawWidth, iconDrawHeight };
            DrawTexturePro(hud->goldTexture, sourceRecGold, destRecGold, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);

            // Desenha o texto do Gold
            DrawText(goldText, textGoldX, textGoldY, fontSize, WHITE);
            currentY += iconDrawHeight + 10.0f; // Move o Y para o próximo item (se houvesse)
        }
    }
}

// --- FUNÇÃO DE FINALIZAÇÃO ---
// Descarrega todas as texturas carregadas para o HUD
void UnloadHud(Hud *hud) {
    if (hud->lifeIconTexture.id != 0) UnloadTexture(hud->lifeIconTexture);
    if (hud->energyIconTexture.id != 0) UnloadTexture(hud->energyIconTexture);
    if (hud->shurikenTexture.id != 0) UnloadTexture(hud->shurikenTexture);
    if (hud->shieldTexture.id != 0) UnloadTexture(hud->shieldTexture);
    // Descarrega Textura do Gold (Ouro)
    if (hud->goldTexture.id != 0) UnloadTexture(hud->goldTexture);
}