#include "b2_bullet.h"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdbool.h>

// --- CONSTANTES DE ESCALA E CAMINHOS ---
#define BULLET_SCALE_WEAK 0.08f
#define BULLET_SCALE_MEDIUM 0.12f
#define BULLET_SCALE_STRONG 0.16f
#define SHURIKEN_SCALE 0.1f

#define WEAK_SPRITE_PATH "assets/byte2/images/sprites/ataque_fraco.png"
#define MEDIUM_SPRITE_PATH "assets/byte2/images/sprites/ataque_medio.png"
#define STRONG_SPRITE_PATH "assets/byte2/images/sprites/ataque_forte.png"
#define SHURIKEN_SPRITE_PATH "assets/byte2/images/sprites/shurikens_byte.png"

// --- CONSTANTES DE SHURIKEN ---
#define SHURIKEN_BASE_SPEED 550.0f
#define SHURIKEN_OFFSET 25.0f // Espaçamento horizontal em relação ao centro
#define SHURIKEN_ANGLE 10.0f // Ângulo de desvio lateral em graus

// --- FUNÇÃO AUXILIAR: Disparo de uma única bala ---
static void FireBullet(BulletManager *manager, Vector2 position, Vector2 speed, float scale, int type, Texture2D texture) {
    // Procura por uma bala inativa no array
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!manager->bullets[i].active) {
            Bullet *bullet = &manager->bullets[i];

            bullet->active = true;
            bullet->type = type;

            bullet->speed = speed;

            // Define o tamanho da caixa de colisão/desenho baseado na escala
            bullet->rect.width = (float)texture.width * scale;
            bullet->rect.height = (float)texture.height * scale;

            // Centraliza o retângulo na posição inicial
            bullet->rect.x = position.x - (bullet->rect.width / 2);
            bullet->rect.y = position.y - (bullet->rect.height / 2);

            return; // Sai após disparar a bala
        }
    }
}

// --- INICIALIZAÇÃO ---
void InitBulletManager(BulletManager *manager) {
    // Carrega as texturas dos projéteis
    manager->weakTexture = LoadTexture(WEAK_SPRITE_PATH);
    manager->mediumTexture = LoadTexture(MEDIUM_SPRITE_PATH);
    manager->strongTexture = LoadTexture(STRONG_SPRITE_PATH);
    manager->shurikenTexture = LoadTexture(SHURIKEN_SPRITE_PATH);

    // Aplica filtro de ponto para manter a qualidade de pixel art
    if (manager->weakTexture.id != 0) SetTextureFilter(manager->weakTexture, TEXTURE_FILTER_POINT);
    if (manager->mediumTexture.id != 0) SetTextureFilter(manager->mediumTexture, TEXTURE_FILTER_POINT);
    if (manager->strongTexture.id != 0) SetTextureFilter(manager->strongTexture, TEXTURE_FILTER_POINT);
    if (manager->shurikenTexture.id != 0) SetTextureFilter(manager->shurikenTexture, TEXTURE_FILTER_POINT);

    // Inicializa todas as balas como inativas
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        manager->bullets[i].active = false;
        manager->bullets[i].color = WHITE;
        manager->bullets[i].speed = (Vector2){ 0, 0 };
    }
}

// --- DISPARO DE ATAQUE CARREGADO ---
void ShootChargedAttack(BulletManager *manager, Vector2 playerCenter, float playerHeight, int attackType, bool hasShurikens) {
    Texture2D mainTexture;
    float mainSpeed = 0.0f;
    float mainScale = 0.0f;

    // Variável de controle: desativa shurikens para tipos 2 e 3
    bool shouldFireShurikens = hasShurikens;

    // 1. Configuração do Projétil Principal (Weak, Medium ou Strong)
    switch (attackType) {
        case ATTACK_STRONG:
            mainTexture = manager->strongTexture;
            mainSpeed = 700.0f;
            mainScale = BULLET_SCALE_STRONG;
            shouldFireShurikens = false; // Ataque FORTE não dispara shurikens
            break;
        case ATTACK_MEDIUM:
            mainTexture = manager->mediumTexture;
            mainSpeed = 600.0f;
            mainScale = BULLET_SCALE_MEDIUM;
            shouldFireShurikens = false; // Ataque MÉDIO não dispara shurikens
            break;
        case ATTACK_WEAK: default:
            mainTexture = manager->weakTexture;
            mainSpeed = 500.0f;
            mainScale = BULLET_SCALE_WEAK;
            // Se for ATTACK_WEAK, shouldFireShurikens mantém o valor original (depende do upgrade)
            break;
    }

    // Posição de disparo (do topo da nave, aproximadamente)
    Vector2 mainPosition = { playerCenter.x, playerCenter.y - (playerHeight / 2) };
    Vector2 mainSpeedVec = { 0, -mainSpeed }; // Vertical para cima

    // Dispara o projétil principal
    FireBullet(manager, mainPosition, mainSpeedVec, mainScale, attackType, mainTexture);

    // 2. Configuração e Disparo dos Shurikens (se permitido)
    if (shouldFireShurikens) {
        float shurikenSpeed = SHURIKEN_BASE_SPEED;

        Vector2 startPosition = { playerCenter.x, playerCenter.y - (playerHeight / 2) };

        // Vetor base: velocidade para cima (Y negativo)
        Vector2 baseSpeedVector = { 0.0f, -shurikenSpeed };

        // Shuriken Esquerdo
        Vector2 pos1 = { startPosition.x - SHURIKEN_OFFSET, startPosition.y };
        // Rotaciona o vetor de velocidade 10 graus para a esquerda (negativo)
        Vector2 speed1 = Vector2Rotate(baseSpeedVector, -SHURIKEN_ANGLE * DEG2RAD);
        FireBullet(manager, pos1, speed1, SHURIKEN_SCALE, ATTACK_SHURIKEN, manager->shurikenTexture);

        // Shuriken Direito
        Vector2 pos2 = { startPosition.x + SHURIKEN_OFFSET, startPosition.y };
        // Rotaciona o vetor de velocidade 10 graus para a direita (positivo)
        Vector2 speed2 = Vector2Rotate(baseSpeedVector, SHURIKEN_ANGLE * DEG2RAD);
        FireBullet(manager, pos2, speed2, SHURIKEN_SCALE, ATTACK_SHURIKEN, manager->shurikenTexture);
    }
}

// --- ATUALIZAÇÃO ---
void UpdatePlayerBullets(BulletManager *manager, float deltaTime) {
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (manager->bullets[i].active) {
            Bullet *bullet = &manager->bullets[i];

            // Atualiza a posição (X += VelocidadeX * dT, Y += VelocidadeY * dT)
            bullet->rect.x += bullet->speed.x * deltaTime;
            bullet->rect.y += bullet->speed.y * deltaTime;

            // Desativa a bala se sair da tela (acima ou nas laterais)
            if (bullet->rect.y < -bullet->rect.height ||
                bullet->rect.x < -bullet->rect.width ||
                bullet->rect.x > GetScreenWidth()) {
                bullet->active = false;
            }
        }
    }
}

// --- DESENHO ---
void DrawPlayerBullets(BulletManager *manager) {
    Texture2D currentTexture;

    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (manager->bullets[i].active) {
            Bullet *bullet = &manager->bullets[i];

            // Seleciona a textura correta
            switch (bullet->type) {
                case ATTACK_STRONG: currentTexture = manager->strongTexture; break;
                case ATTACK_MEDIUM: currentTexture = manager->mediumTexture; break;
                case ATTACK_SHURIKEN: currentTexture = manager->shurikenTexture; break;
                case ATTACK_WEAK: default: currentTexture = manager->weakTexture; break;
            }

            Rectangle sourceRec = { 0.0f, 0.0f, (float)currentTexture.width, (float)currentTexture.height };

            float rotation = 0.0f;

            // Rotação especial para o Shuriken (gira constantemente)
            if (bullet->type == ATTACK_SHURIKEN) {
                rotation = fmod(GetTime() * 500.0f, 360.0f);
            }

            // O centro da textura para rotação e desenho
            Vector2 origin = { bullet->rect.width / 2.0f, bullet->rect.height / 2.0f };

            // Desenha a textura usando DrawTexturePro para aplicar escala, rotação e origem
            DrawTexturePro(
                currentTexture,
                sourceRec,
                (Rectangle){
                    // Posição ajustada: Raylib Pro desenha o centro da textura no X/Y
                    bullet->rect.x + origin.x,
                    bullet->rect.y + origin.y,
                    bullet->rect.width,
                    bullet->rect.height
                },
                origin,
                rotation,
                WHITE
            );
        }
    }
}

// --- DESCARREGAMENTO ---
void UnloadBulletManager(BulletManager *manager) {
    if (manager->weakTexture.id != 0) UnloadTexture(manager->weakTexture);
    if (manager->mediumTexture.id != 0) UnloadTexture(manager->mediumTexture);
    if (manager->strongTexture.id != 0) UnloadTexture(manager->strongTexture);
    if (manager->shurikenTexture.id != 0) UnloadTexture(manager->shurikenTexture);
}