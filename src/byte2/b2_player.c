#include "b2_player.h"
#include "raylib.h"
#include "raymath.h"
#include "b2_audio.h"
#include "b2_bullet.h"
#include <stdio.h>

// --- CONSTANTES DE MOVIMENTO E LIMITES ---
#define PLAYER_MOVE_SPEED 400.0f       // Velocidade de movimento em pixels/segundo
#define BLACK_AREA_START_Y 480.0f    // Limite superior onde o jogador pode se mover (área escura do mapa)

// --- CONSTANTES DE CARGA DE ENERGIA ---
#define CHARGE_RATE 14.286f          // Taxa de aumento de energia por segundo (aprox. 100/7 segundos para carga máxima)
#define MAX_CHARGE 100.0f            // Carga máxima (100%)
#define MEDIUM_THRESHOLD 50.0f       // Limite para o ataque de nível Médio (50%)

// --- CONSTANTES DE EFEITO AURA (Tiro Carregado) ---
#define AURA_MAX_RADIUS_FACTOR 0.8f  // Fator de aumento máximo do raio da aura (em relação à nave)
#define AURA_MIN_ALPHA 0.3f          // Transparência mínima da aura (pulsando)
#define AURA_MAX_ALPHA 0.8f          // Transparência máxima da aura (pulsando)
#define AURA_PULSE_SPEED 5.0f        // Velocidade da pulsação da aura
#define FIRE_AURA_RADIUS_INCREASE 1.1f // Aumento de raio para a aura do tiro Forte (MAX_CHARGE)

// --- CAMINHOS DAS TEXTURAS/SPRITES ---
#define BASE_SPRITE_PATH "assets/byte2/images/sprites/byte_1.png"
#define SHURIKEN_SPRITE_PATH "assets/byte2/images/sprites/byte_2.png"         // Sprite com power-up de Double Shot
#define SHIELD_SPRITE_PATH "assets/byte2/images/sprites/byte_shield.png"     // Sprite com power-up de Escudo
#define EXTRA_LIFE_SPRITE_PATH "assets/byte2/images/sprites/byte_4.png"      // Sprite de Vida Extra (Removido, mas mantido)

// --- CORES DA AURA (Tiro Carregado) ---
#define COLOR_WEAK (CLITERAL(Color){ 0, 191, 255, 255 })  // Ciano/Azul claro (Ataque Fraco)
#define COLOR_MEDIUM (CLITERAL(Color){ 128, 0, 255, 255 }) // Roxo (Ataque Médio)
#define COLOR_FIRE (CLITERAL(Color){ 255, 69, 0, 255 })   // Laranja avermelhado (Ataque Forte)

// --- FUNÇÃO PARA CALCULAR O TIPO DE ATAQUE ---
int CalculateAttackType(float charge) {
    if (charge >= MAX_CHARGE) {
        return ATTACK_STRONG; // Ataque Forte (100%)
    }
    else if (charge >= MEDIUM_THRESHOLD) {
        return ATTACK_MEDIUM; // Ataque Médio (50% a 99%)
    }
    else {
        return ATTACK_WEAK; // Ataque Fraco (0% a 49%)
    }
}

// --- FUNÇÃO DE INICIALIZAÇÃO DO JOGADOR ---
void InitPlayer(Player *player) {
    // Carrega as diferentes texturas do jogador
    player->baseTexture = LoadTexture(BASE_SPRITE_PATH);
    player->shurikenTexture = LoadTexture(SHURIKEN_SPRITE_PATH);
    player->shieldTextureAppearance = LoadTexture(SHIELD_SPRITE_PATH);
    player->extraLifeTextureAppearance = LoadTexture(EXTRA_LIFE_SPRITE_PATH);

    // Inicialmente, usa a textura base
    player->texture = player->baseTexture;

    player->scale = PLAYER_SCALE;
    player->speed = PLAYER_MOVE_SPEED;

    player->gold = 0;

    // Inicializa a vida do jogador como 3
    player->maxLives = 3;
    player->currentLives = 3;

    // --- ESTADO DO TIRO CARREGADO ---
    player->energyCharge = 0.0f;
    player->isCharging = false;
    player->canCharge = false; // Começa desabilitado (precisa comprar upgrade)

    // --- POWER-UPS ---
    player->hasDoubleShot = false;
    player->hasShield = false;
    player->extraLives = 0;

    // Cálculo do tamanho escalado
    float player_width_scaled = player->texture.width * player->scale;
    float player_height_scaled = (float)player->texture.height * player->scale;

    // Posição inicial (no centro, próximo ao fundo)
    player->position = (Vector2){
        (float)GetScreenWidth()/2 - player_width_scaled/2,
        (float)GetScreenHeight() - player_height_scaled - 10.0f
    };

    // Aplica o filtro POINT (pixel art) a todas as texturas carregadas
    if (player->baseTexture.id != 0) SetTextureFilter(player->baseTexture, TEXTURE_FILTER_POINT);
    if (player->shurikenTexture.id != 0) SetTextureFilter(player->shurikenTexture, TEXTURE_FILTER_POINT);
    if (player->shieldTextureAppearance.id != 0) SetTextureFilter(player->shieldTextureAppearance, TEXTURE_FILTER_POINT);
    if (player->extraLifeTextureAppearance.id != 0) SetTextureFilter(player->extraLifeTextureAppearance, TEXTURE_FILTER_POINT);

    // Inicialização da Aura
    player->auraRadius = player_width_scaled * 0.5f;
    player->auraAlpha = 0.0f;
    player->auraPulseSpeed = AURA_PULSE_SPEED;

}

// --- FUNÇÃO DE ATUALIZAÇÃO DO JOGADOR ---
void UpdatePlayer(Player *player, BulletManager *bulletManager, AudioManager *audioManager, Hud *hud, float deltaTime, int screenWidth, int screenHeight) {
    float move_dist = player->speed * deltaTime;

    // --- MOVIMENTO ---
    if (IsKeyDown(KEY_LEFT)) player->position.x -= move_dist;
    if (IsKeyDown(KEY_RIGHT)) player->position.x += move_dist;
    if (IsKeyDown(KEY_UP)) player->position.y -= move_dist;
    if (IsKeyDown(KEY_DOWN)) player->position.y += move_dist;

    float ship_width = player->texture.width * player->scale;
    float ship_height = player->texture.height * player->scale;
    Vector2 playerCenter = {
        player->position.x + ship_width / 2,
        player->position.y + ship_height / 2
    };

    // Apenas executa a lógica de tiro se os gerenciadores estiverem disponíveis
    if (audioManager != NULL && bulletManager != NULL) {
        // --- LÓGICA DE CARREGAMENTO (CHARGE) ---
        if (IsKeyDown(KEY_SPACE)) {
            player->isCharging = true;
            // Aumenta a carga de energia (Clamped entre 0 e MAX_CHARGE)
            player->energyCharge = Clamp(player->energyCharge + CHARGE_RATE * deltaTime, 0.0f, MAX_CHARGE);

            // Toca o som de carregamento (se não estiver tocando)
            if (!IsSoundPlaying(audioManager->sfxCharge)) {
                PlaySound(audioManager->sfxCharge);
            }

            // --- CÁLCULO DA AURA (RAIO E ALPHA) ---
            float baseRadius = ship_width / 2.0f;
            // Calcula o quanto o raio pode aumentar
            float maxIncrease = (ship_width * AURA_MAX_RADIUS_FACTOR) - baseRadius;
            // O raio aumenta proporcionalmente à carga
            player->auraRadius = baseRadius + (maxIncrease * (player->energyCharge / MAX_CHARGE));

            // Efeito de pulsação no alpha usando a função seno
            float pulseFactor = (sin(GetTime() * player->auraPulseSpeed) * 0.5f + 0.5f);
            player->auraAlpha = AURA_MIN_ALPHA + pulseFactor * (AURA_MAX_ALPHA - AURA_MIN_ALPHA);

        }
        // --- LÓGICA DE DISPARO (RELEASE) ---
        else if (IsKeyReleased(KEY_SPACE) && player->energyCharge > 0.0f) {
            player->isCharging = false;

            // Determina o tipo de ataque com base na carga
            int attackType = CalculateAttackType(player->energyCharge);

            // Cria a(s) bala(s) no BulletManager
            // O status do power-up (hasDoubleShot) é passado para a função de tiro
            ShootChargedAttack(bulletManager, playerCenter, ship_height, attackType, player->hasDoubleShot);

            // Toca o som de ataque correspondente
            PlayAttackSfx(audioManager, attackType);

            // Reseta a carga
            player->energyCharge = 0.0f;
        }

        // --- LÓGICA DE NÃO-CARREGAMENTO (NOT PRESSING SPACE) ---
        if (!IsKeyDown(KEY_SPACE)) {
            // Se soltou a tecla, mas não tinha carga suficiente (ou já disparou), reseta
            player->isCharging = false;
            // Garante que o som de carga pare
            StopSound(audioManager->sfxCharge);

            // Nota: Se a energia era > 0.0f, ela já foi resetada no 'IsKeyReleased'
        }
    }

    // --- LIMITES DE TELA ---
    // Clamped X: Limita o jogador dentro das bordas horizontais
    player->position.x = Clamp(player->position.x, 0.0f, (float)screenWidth - ship_width);
    // Clamped Y: Limita o jogador entre a área escura (BLACK_AREA_START_Y) e o fundo da tela
    player->position.y = Clamp(player->position.y, BLACK_AREA_START_Y, (float)screenHeight - ship_height);
}

// --- FUNÇÃO DE DESENHO DO JOGADOR ---
void DrawPlayer(Player *player) {
    if (player->texture.id == 0) return; // Não desenha se a textura não estiver carregada

    float ship_width = player->texture.width * player->scale;
    float ship_height = player->texture.height * player->scale;
    Vector2 playerCenter = {
        player->position.x + ship_width / 2,
        player->position.y + ship_height / 2
    };

    // --- EFEITO DE MOTOR/PROPULSÃO (FOGO) ---
    float firePulse = (sin(GetTime() * 20.0f) * 0.1f + 0.9f); // Pulsação rápida do fogo
    float fireRadius = ship_width * 0.08f / player->scale * firePulse; // Raio do fogo

    // Posição do fogo (na parte inferior da nave)
    Vector2 firePos = { playerCenter.x, player->position.y + ship_height - (ship_height / 10.0f) };

    // Desenha o fogo (gradientes de círculos para dar efeito de glow)
    DrawCircleGradient(
        (int)firePos.x, (int)firePos.y, fireRadius * 1.5f, // Raio maior (sombra)
        Fade(RED, 0.5f), Fade(RED, 0.0f)
    );
    DrawCircleGradient(
        (int)firePos.x, (int)firePos.y, fireRadius, // Raio menor (núcleo)
        Fade(YELLOW, 0.9f), Fade(RED, 0.0f)
    );

    // --- EFEITO DE AURA (ENQUANTO CARREGANDO) ---
    if (player->isCharging) {
        Color baseColor;
        bool isMaxCharge = (player->energyCharge >= MAX_CHARGE);

        // Define a cor base da aura de acordo com a carga
        if (player->energyCharge >= MEDIUM_THRESHOLD) {
            baseColor = COLOR_MEDIUM;
        } else {
            baseColor = COLOR_WEAK;
        }

        // Cor da aura com o alpha pulsante
        Color auraColor = {
            baseColor.r,
            baseColor.g,
            baseColor.b,
            (unsigned char)(player->auraAlpha * 255.0f)
        };

        // Desenha a Aura do Tiro Forte (MAX_CHARGE) - Efeito de Fogo
        if (isMaxCharge) {
            Color fireColor = {
                COLOR_FIRE.r,
                COLOR_FIRE.g,
                COLOR_FIRE.b,
                (unsigned char)(player->auraAlpha * 255.0f)
            };

            // Desenha um círculo maior e vermelho/laranja por baixo
            DrawCircleGradient(
                (int)playerCenter.x,
                (int)playerCenter.y,
                player->auraRadius * FIRE_AURA_RADIUS_INCREASE,
                Fade(fireColor, 0.7f),
                Fade(fireColor, 0.0f)
            );
        }

        // Desenha a Aura Principal (que cresce e pulsa)
        DrawCircleGradient(
            (int)playerCenter.x,
            (int)playerCenter.y,
            player->auraRadius,
            Fade(auraColor, 0.8f),
            Fade(auraColor, 0.0f)
        );
    }

    // --- DESENHO DA NAVE (SPRITE) ---
    DrawTextureEx(player->texture, player->position, 0.0f, player->scale, WHITE);
}

// --- FUNÇÃO DE FINALIZAÇÃO ---
// Descarrega todas as texturas da memória
void UnloadPlayer(Player *player) {
    if (player->baseTexture.id != 0) UnloadTexture(player->baseTexture);
    if (player->shurikenTexture.id != 0) UnloadTexture(player->shurikenTexture);
    if (player->shieldTextureAppearance.id != 0) UnloadTexture(player->shieldTextureAppearance);
    if (player->extraLifeTextureAppearance.id != 0) UnloadTexture(player->extraLifeTextureAppearance);
}