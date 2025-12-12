#include "b2_enemy.h"
#include "raylib.h"
#include "raymath.h"
#include "b2_audio.h"
#include "b2_bullet.h"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define COLOR_NEON_BLUE (CLITERAL(Color){ 0, 191, 255, 255 })
#define COLOR_NEON_PURPLE (CLITERAL(Color){ 128, 0, 255, 255 })
#define COLOR_NEON_RED (CLITERAL(Color){ 255, 69, 0, 255 })
#define COLOR_EXPLOSION_ORANGE (CLITERAL(Color){ 255, 165, 0, 255 })
#define COLOR_NEON_GREEN (CLITERAL(Color){ 0, 255, 0, 255 })

#define EXPLOSION_PARTICLE_COUNT 100

#define WAVE_START_DURATION 3.0f
#define ENEMY_INITIAL_HEALTH_T1 1
#define ENEMY_INITIAL_HEALTH_T2 2
#define ENEMY_INITIAL_HEALTH_T3 4

// NOVA CONSTANTE PARA CONTROLE DA DESCIDA DO BOSS (SUGERIDO: Mova para enemy.h)
#define BOSS_DROP_AMOUNT 30.0f

// Array de caminhos para os frames do Boss (corrigido para caminhos relativos)
static const char *bossFramePaths[BOSS_FRAME_COUNT] = {
    "assets/byte2/images/sprites/frame_04_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_05_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_06_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_07_delay-008s.gif",
    "assets/byte2/images/sprites/frame_08_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_09_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_10_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_11_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_12_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_13_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_14_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_15_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_16_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_17_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_18_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_19_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_20_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_21_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_22_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_23_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_24_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_25_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_26_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_27_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_28_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_00_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_01_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_02_delay-0.08s.gif",
    "assets/byte2/images/sprites/frame_03_delay-0.08s.gif",
};

// --- Funções de Partículas  ---

void InitParticleManager(ParticleManager *manager) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        manager->particles[i].active = false;
    }
    manager->nextParticleIndex = 0;
}

Color GetRandomNeonColor() {
    int r = GetRandomValue(0, 5);
    switch (r) {
        case 0: return RED;
        case 1: return COLOR_EXPLOSION_ORANGE;
        case 2: return YELLOW;
        case 3: return VIOLET;
        case 4: return COLOR_NEON_BLUE;
        case 5: return LIME;
        default: return WHITE;
    }
}

void ExplodeEnemy(EnemyManager *manager, Vector2 position, int particleCount) {
    ParticleManager *pm = &manager->particleManager;
    for (int i = 0; i < particleCount; i++) {
        Particle *p = &pm->particles[pm->nextParticleIndex];

        p->active = true;
        p->position = position;
        p->life = PARTICLE_LIFESPAN;

        float speed = (float)GetRandomValue(150, 400);
        float angle = (float)GetRandomValue(0, 359);

        p->velocity.x = cosf(angle * DEG2RAD) * speed;
        p->velocity.y = sinf(angle * DEG2RAD) * speed;

        p->color = GetRandomNeonColor();

        pm->nextParticleIndex = (pm->nextParticleIndex + 1) % MAX_PARTICLES;
    }
}

void UpdateParticles(ParticleManager *manager, float deltaTime) {
    const int GAME_WIDTH = 800;
    const int GAME_HEIGHT = 600;

    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &manager->particles[i];
        if (!p->active) continue;

        p->life -= deltaTime;

        if (p->life <= 0.0f) {
            p->active = false;
        } else {
            p->position.x += p->velocity.x * deltaTime;
            p->position.y += p->velocity.y * deltaTime;

            if (p->position.x < 0 || p->position.x > GAME_WIDTH) {
                p->velocity.x *= -1;
                p->position.x = fmaxf(0, fminf(p->position.x, (float)GAME_WIDTH));
            }
            if (p->position.y < 0 || p->position.y > GAME_HEIGHT) {
                p->velocity.y *= -1;
                p->position.y = fmaxf(0, fminf(p->position.y, (float)GAME_HEIGHT));
            }

            p->velocity = Vector2Scale(p->velocity, 1.0f - (0.8f * deltaTime));
        }
    }
}

void DrawParticles(ParticleManager *manager) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &manager->particles[i];
        if (!p->active) continue;

        float alpha = p->life / PARTICLE_LIFESPAN;
        Color drawColor = Fade(p->color, alpha);

        DrawCircleV(p->position, 3.0f, drawColor);
    }
}

// --- Funções de Inimigo Normal  ---

void DrawEnemy(Enemy *enemy, Texture2D texture) {
    if (!enemy->active) return;

    const float BASE_AURA_RADIUS = ENEMY_SIZE * 0.8f;
    float time = (float)GetTime();
    float pulse = (sinf(time * 6.0f) + 1.0f) * 0.5f;
    float currentRadius = BASE_AURA_RADIUS + (pulse * 2.0f);

    Color outerColor = Fade(enemy->neonColor, 0.15f + (pulse * 0.05f));
    DrawRing(enemy->position, currentRadius * 0.85f, currentRadius * 1.0f, 0, 360, 30, outerColor);

    Color middleColor = Fade(enemy->neonColor, 0.4f);
    DrawRing(enemy->position, currentRadius * 0.75f, currentRadius * 0.85f, 0, 360, 30, middleColor);

    Color innerColor = Fade(WHITE, 0.6f);
    DrawRing(enemy->position, currentRadius * 0.7f, currentRadius * 0.75f, 0, 360, 30, innerColor);

    float w = ENEMY_SIZE;
    float h = ENEMY_SIZE;

    Rectangle destRec = { enemy->position.x - w / 2, enemy->position.y - h / 2, w, h };
    Vector2 origin = { 0.0f, 0.0f };

    Color tintColor = WHITE;
    if (enemy->hitTimer > 0.0f) {
        tintColor = RED;
    }

    DrawTexturePro(
        texture,
        (Rectangle){ 0.0f, 0.0f, (float)texture.width, (float)texture.height },
        destRec,
        origin,
        0.0f,
        tintColor
    );
}

void DrawExplosion(Enemy *enemy) {
    float progress = 1.0f - (enemy->explosionTimer / ENEMY_EXPLOSION_DURATION);
    float maxRadius = ENEMY_SIZE * 1.5f;
    float currentRadius = maxRadius * progress;

    Color outerColor = Fade(COLOR_EXPLOSION_ORANGE, 1.0f - progress);
    DrawCircle(
        (int)enemy->position.x,
        (int)enemy->position.y,
        currentRadius,
        outerColor
    );

    Color centerColor = Fade(YELLOW, 1.0f - progress * 0.5f);
    DrawCircle(
        (int)enemy->position.x,
        (int)enemy->position.y,
        currentRadius * 0.6f,
        centerColor
    );
}

// --- Funções do Boss  ---

void InitBoss(Boss *boss, int screenWidth, int screenHeight) {
    boss->active = true;
    boss->health = BOSS_INITIAL_HEALTH;
    boss->maxHealth = BOSS_INITIAL_HEALTH;
    boss->hitTimer = 0.0f;

    // Posição inicial (centro superior)
    boss->position.x = (float)screenWidth / 2.0f;
    boss->position.y = 100.0f;

    // Colisão
    boss->rect.width = BOSS_SIZE_WIDTH;
    boss->rect.height = BOSS_SIZE_HEIGHT;
    boss->rect.x = boss->position.x - BOSS_SIZE_WIDTH / 2.0f;
    boss->rect.y = boss->position.y - BOSS_SIZE_HEIGHT / 2.0f;

    // Animação
    boss->currentFrame = 0;
    boss->frameTimer = BOSS_ANIMATION_SPEED;

    // Movimento inicial (Ex: centro-direita)
    boss->movementTimer = 0.0f;
    boss->targetPosition.x = (float)screenWidth * 0.75f;
    // targetPosition.y é o que controla a descida progressiva
    boss->targetPosition.y = 100.0f;
}

void UpdateBoss(Boss *boss, float deltaTime, int screenWidth) {
    if (!boss->active) return;

    // 1. Lógica de Animação
    boss->frameTimer -= deltaTime;
    if (boss->frameTimer <= 0.0f) {
        boss->currentFrame = (boss->currentFrame + 1) % BOSS_FRAME_COUNT;
        boss->frameTimer = BOSS_ANIMATION_SPEED;
    }

    // 2. Lógica de Hit
    if (boss->hitTimer > 0.0f) {
        boss->hitTimer -= deltaTime;
    }

    // 3. Lógica de Movimento (Movimento pendular simples com descida)
    float speed = 100.0f;
    Vector2 direction = Vector2Subtract(boss->targetPosition, boss->position);
    float distance = Vector2Length(direction);

    if (distance > 5.0f) {
        // Movendo em direção ao alvo
        Vector2 normalizedDirection = Vector2Normalize(direction);
        boss->position.x += normalizedDirection.x * speed * deltaTime;
        boss->position.y += normalizedDirection.y * speed * deltaTime;

    } else {
        // Chegou ao alvo lateral, define um novo alvo
        float currentX = boss->position.x;

        // Aumenta a posição Y do alvo para descer em direção ao jogador
        boss->targetPosition.y += BOSS_DROP_AMOUNT;

        if (currentX < screenWidth / 2.0f) {
            // Mudar para o lado direito
            boss->targetPosition.x = (float)screenWidth * 0.75f;
        } else {
            // Mudar para o lado esquerdo
            boss->targetPosition.x = (float)screenWidth * 0.25f;
        }
    }

    // Atualiza o retângulo de colisão
    boss->rect.x = boss->position.x - boss->rect.width / 2.0f;
    boss->rect.y = boss->position.y - boss->rect.height / 2.0f;
}

void DrawBoss(Boss *boss, Texture2D frames[]) {
    if (!boss->active) return;

    Texture2D currentTexture = frames[boss->currentFrame];

    float w = BOSS_SIZE_WIDTH;
    float h = BOSS_SIZE_HEIGHT;

    // DestRec para centralizar no ponto boss->position
    Rectangle destRec = { boss->position.x, boss->position.y, w, h };
    Vector2 origin = { w / 2.0f, h / 2.0f };

    Color tintColor = WHITE;
    if (boss->hitTimer > 0.0f) {
        tintColor = RED;
    }

    DrawTexturePro(
        currentTexture,
        (Rectangle){ 0.0f, 0.0f, (float)currentTexture.width, (float)currentTexture.height },
        destRec,
        origin,
        0.0f,
        tintColor
    );

    // Desenhar barra de vida do Boss
    float barWidth = w * 1.5f;
    float barHeight = 10.0f;
    float healthRatio = (float)boss->health / (float)boss->maxHealth;

    Rectangle healthBarBg = {
        boss->position.x - barWidth / 2.0f,
        boss->position.y - h / 2.0f - barHeight - 10.0f,
        barWidth,
        barHeight
    };

    DrawRectangleRec(healthBarBg, GRAY);
    DrawRectangle(
        (int)healthBarBg.x,
        (int)healthBarBg.y,
        (int)(barWidth * healthRatio),
        (int)barHeight,
        LIME
    );
}

// --- Lógica de Inicialização da Onda (Mantida) ---

void InitEnemiesForWave(EnemyManager *manager, int screenWidth, int screenHeight, int waveNumber) {

    // 1. Configurações do Boss
    if (waveNumber >= 10 && !manager->bossActive) {
        manager->bossActive = true;
        InitBoss(&manager->boss, screenWidth, screenHeight);
    } else if (waveNumber < 10) {
        manager->bossActive = false;
        manager->boss.active = false;
    }

    // 2. Configurações de Inimigos Normais

    float speedMultiplier = 1.0f + (waveNumber - 1) * 0.15f;
    int healthBoost = (waveNumber - 1) / 3;
    float startY = 20.0f;
    int enemiesToActivate = ENEMY_COUNT;
    int offsetRows = 0;

    if (waveNumber >= 10) {
        // Reduz a velocidade lateral na fase do boss e usa a dificuldade da wave 9
        speedMultiplier = 1.0f + (9 - 1) * 0.15f;
        manager->speed = ENEMY_SPEED_INITIAL * speedMultiplier * 0.5f;

        healthBoost = (9 - 1) / 3;

        // Inicia mais para baixo para evitar o Boss
        startY = 200.0f;

        // REMOVER PRIMEIRA LINHA (Linha 0)
        enemiesToActivate = ENEMY_COUNT - ENEMY_COLS;
        offsetRows = 1;
    } else {
        // Waves normais (antes da 10)
        manager->speed = ENEMY_SPEED_INITIAL * speedMultiplier;
    }

    float totalWidth = ENEMY_COLS * ENEMY_SIZE + (ENEMY_COLS - 1) * ENEMY_PADDING_X;
    float startX = (screenWidth - totalWidth) / 2.0f;

    manager->direction = 1;
    manager->activeCount = enemiesToActivate;
    manager->gameOver = false;
    manager->gameHeight = screenHeight;

    const float COLLISION_MARGIN = 5.0f;
    const float COLLISION_DIMENSION = ENEMY_SIZE + COLLISION_MARGIN * 2;

    for (int i = 0; i < ENEMY_COUNT; i++) {
        Enemy *enemy = &manager->enemies[i];
        int row = i / ENEMY_COLS;
        int col = i % ENEMY_COLS;

        // Lógica para desativar a primeira linha na wave do Boss
        if (waveNumber >= 10 && row == 0) {
            enemy->active = false;
            enemy->health = 0;
            continue;
        }

        // Se o Boss estiver ativo, ajustamos a linha inicial
        int effectiveRow = row - offsetRows;
        if (effectiveRow < 0) effectiveRow = 0;

        // Posição y ajustada: a primeira linha visível usa startY
        float currentY = startY + effectiveRow * (ENEMY_SIZE + ENEMY_PADDING_Y) + ENEMY_SIZE / 2.0f;

        enemy->position.x = startX + col * (ENEMY_SIZE + ENEMY_PADDING_X) + ENEMY_SIZE / 2.0f;
        enemy->position.y = currentY;

        enemy->rect.width = COLLISION_DIMENSION;
        enemy->rect.height = COLLISION_DIMENSION;
        enemy->rect.x = enemy->position.x - COLLISION_DIMENSION / 2.0f;
        enemy->rect.y = enemy->position.y - COLLISION_DIMENSION / 2.0f;

        enemy->active = true;
        enemy->hitTimer = 0.0f;
        enemy->isExploding = false;
        enemy->explosionTimer = 0.0f;

        // Definição de tipo/saúde (usa a linha original para definir o tipo)
        if (row == 0) {
            enemy->neonColor = COLOR_NEON_RED;
            enemy->type = 3;
            enemy->health = ENEMY_INITIAL_HEALTH_T3 + healthBoost;
        } else if (row <= 2) {
            enemy->neonColor = COLOR_NEON_PURPLE;
            enemy->type = 2;
            enemy->health = ENEMY_INITIAL_HEALTH_T2 + healthBoost;
        } else {
            enemy->neonColor = COLOR_NEON_BLUE;
            enemy->type = 1;
            enemy->health = ENEMY_INITIAL_HEALTH_T1 + healthBoost;
        }
    }
}

void InitEnemyManager(EnemyManager *manager, int screenWidth, int screenHeight) {
    manager->enemyTextures[0] = LoadTexture("assets/byte2/images/sprites/inimigo_1.png");
    manager->enemyTextures[1] = LoadTexture("assets/byte2/images/sprites/inimigo_2.png");
    manager->enemyTextures[2] = LoadTexture("assets/byte2/images/sprites/inimigo_3.png");

    // Carregamento dos Frames do Boss
    for (int i = 0; i < BOSS_FRAME_COUNT; i++) {
        manager->bossFrames[i] = LoadTexture(bossFramePaths[i]);
    }

    manager->currentWave = 1;
    manager->waveStartTimer = WAVE_START_DURATION;
    manager->gameHeight = screenHeight;

    manager->wavesCompletedCount = 0;
    manager->triggerShopReturn = false;

    manager->bossActive = false;

    InitParticleManager(&manager->particleManager);

    InitEnemiesForWave(manager, screenWidth, screenHeight, manager->currentWave);
}

void CheckWaveCompletion(EnemyManager *manager, int screenWidth, int screenHeight) {
    // A onda termina se o Boss for derrotado E/OU se os inimigos normais acabarem.
    if (!manager->bossActive && manager->activeCount == 0) {
        manager->wavesCompletedCount++;
        if (manager->wavesCompletedCount % 3 == 0) {
            manager->triggerShopReturn = true;
        } else {
            manager->currentWave++;
            manager->waveStartTimer = WAVE_START_DURATION;
            InitEnemiesForWave(manager, screenWidth, screenHeight, manager->currentWave);
        }
    }
}

// --- Lógica de Atualização ---

void UpdateEnemies(EnemyManager *manager, float deltaTime, int screenWidth, int *playerLives, bool *gameOver) {
    int screenHeight = manager->gameHeight;

    if (manager->waveStartTimer > 0.0f) {
        manager->waveStartTimer -= deltaTime;
        UpdateParticles(&manager->particleManager, deltaTime);
        if (manager->waveStartTimer > 0.0f) {
             return;
        }
    }

    if (!manager->triggerShopReturn) {
        CheckWaveCompletion(manager, screenWidth, screenHeight);
    }

    if (manager->triggerShopReturn || manager->waveStartTimer > 0.0f) {
        UpdateParticles(&manager->particleManager, deltaTime);
        return;
    }

    if (manager->gameOver) {
        UpdateParticles(&manager->particleManager, deltaTime);
        return;
    }

    UpdateParticles(&manager->particleManager, deltaTime);

    // --- Lógica de Atualização do Boss ---
    if (manager->bossActive) {
        UpdateBoss(&manager->boss, deltaTime, screenWidth);

        // CHECAGEM: Boss atingiu a linha de Game Over (Hit Kill)
        if (manager->boss.position.y >= ENEMY_GAME_OVER_LINE_Y && manager->boss.active) {

            manager->boss.active = false; // Desativa o Boss
            manager->bossActive = false;

            // Perde todas as vidas
            if (*playerLives > 0) {
                *playerLives = 0;
            }

            manager->gameOver = true;
            (*gameOver) = true;
            return; // O jogo acabou.
        }
    }

    // --- Lógica de Atualização dos Inimigos Normais ---
    float moveAmount = manager->speed * manager->direction * deltaTime;
    bool shouldDrop = false;

    float minX = (float)screenWidth;
    float maxX = 0.0f;

    for (int i = 0; i < ENEMY_COUNT; i++) {
        Enemy *enemy = &manager->enemies[i];

        if (enemy->hitTimer > 0.0f) {
            enemy->hitTimer -= deltaTime;
        }

        if (enemy->isExploding) {
            enemy->explosionTimer -= deltaTime;

            if (enemy->explosionTimer <= 0.0f) {
                enemy->isExploding = false;
            }
        }

        if (!enemy->active && !enemy->isExploding) continue;

        if (enemy->active) {
            minX = fminf(minX, enemy->rect.x);
            maxX = fmaxf(maxX, enemy->rect.x + enemy->rect.width);
        }
    }

    const float margin = 2.0f;

    if (manager->direction == 1) {
        if (maxX >= screenWidth - margin) {
            shouldDrop = true;
        }
    } else {
        if (minX <= margin) {
            shouldDrop = true;
        }
    }

    for (int i = 0; i < ENEMY_COUNT; i++) {
        Enemy *enemy = &manager->enemies[i];
        if (!enemy->active) continue;

        enemy->position.x += moveAmount;
        enemy->rect.x = enemy->position.x - enemy->rect.width / 2.0f;
    }

    if (shouldDrop) {
        manager->direction *= -1;
        manager->speed *= 1.02f;

        for (int i = 0; i < ENEMY_COUNT; i++) {
            Enemy *enemy = &manager->enemies[i];
            if (!enemy->active) continue;

            enemy->position.y += ENEMY_DROP_AMOUNT;
            enemy->rect.y = enemy->position.y - enemy->rect.height / 2.0f;

            if (enemy->position.y >= ENEMY_GAME_OVER_LINE_Y) {

                enemy->active = false;
                manager->activeCount--;

                if (*playerLives > 0) {
                    (*playerLives)--;
                }

                if (*playerLives <= 0) {
                    *playerLives = 0;
                    manager->gameOver = true;
                    (*gameOver) = true;
                }
            }
        }
    }
}

// --- Funções de Desenho e Colisão  ---

void DrawEnemies(EnemyManager *manager) {
    DrawParticles(&manager->particleManager);

    // Desenha o Boss se estiver ativo
    if (manager->bossActive) {
        DrawBoss(&manager->boss, manager->bossFrames);
    }

    // Desenha inimigos normais
    for (int i = 0; i < ENEMY_COUNT; i++) {
        Enemy *enemy = &manager->enemies[i];

        if (enemy->isExploding) {
            DrawExplosion(enemy);

        } else if (enemy->active) {
            Texture2D texture;
            switch (enemy->type) {
                case 1: texture = manager->enemyTextures[0]; break;
                case 2: texture = manager->enemyTextures[1]; break;
                case 3: texture = manager->enemyTextures[2]; break;
                default: texture = manager->enemyTextures[0]; break;
            }
            DrawEnemy(enemy, texture);
        }
    }
}

void UnloadEnemyManager(EnemyManager *manager) {
    for (int i = 0; i < 3; i++) {
        if (manager->enemyTextures[i].id != 0) UnloadTexture(manager->enemyTextures[i]);
    }

    // Descarregar as texturas do Boss
    for (int i = 0; i < BOSS_FRAME_COUNT; i++) {
        if (manager->bossFrames[i].id != 0) UnloadTexture(manager->bossFrames[i]);
    }
}

void CheckBulletEnemyCollision(BulletManager *bulletManager, EnemyManager *enemyManager, int *playerGold, AudioManager *audioManager) {

    // --- Lógica de Colisão do Boss  ---
    Boss *boss = &enemyManager->boss;
    if (enemyManager->bossActive && boss->active) {
        for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
            Bullet *bullet = &bulletManager->bullets[i];

            if (!bullet->active) continue;

            Vector2 bulletCenter = {
                bullet->rect.x + bullet->rect.width / 2.0f,
                bullet->rect.y + bullet->rect.height / 2.0f
            };
            float bulletRadius = bullet->rect.width / 2.0f;

            float bossRadius = BOSS_SIZE_WIDTH / 2.0f;

            // Colisão de Círculos para o Boss
            if (CheckCollisionCircles(bulletCenter, bulletRadius, boss->position, bossRadius * 0.8f)) {

                // Filtro: O Boss SÓ leva dano dos tiros de tipo 2 (Médio) e 3 (Forte).
                if (bullet->type == 1 || bullet->type == 0 || bullet->type == 4) {
                    // Bala destruída, mas Boss não leva dano
                    bullet->active = false;
                    PlaySound(audioManager->sfxWeak);
                    continue;
                }

                // Aplica NOVO DANO
                int damage = 0;
                if (bullet->type == 2) {
                    damage = 750; //  Dano Médio
                } else if (bullet->type == 3) {
                    damage = 1500; //  Dano Forte
                }

                if (damage > 0) {
                    bullet->active = false;

                    boss->health -= damage; // Aplica o dano
                    boss->hitTimer = ENEMY_FLASH_DURATION; // Boss pisca em vermelho
                    PlaySound(audioManager->sfxWeak);

                    if (boss->health <= 0) {
                        boss->active = false;
                        enemyManager->bossActive = false;

                        *playerGold += 100;

                        PlaySound(audioManager->sfxExplosionEnemy);
                        ExplodeEnemy(enemyManager, boss->position, EXPLOSION_PARTICLE_COUNT * 5);

                        if (enemyManager->activeCount == 0) {
                            enemyManager->wavesCompletedCount++;
                            enemyManager->triggerShopReturn = true;
                        }
                    }
                }
                break;
            }
        }
    }

    // --- Lógica de Colisão de Inimigos Normais  ---
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        Bullet *bullet = &bulletManager->bullets[i];

        if (!bullet->active) continue;

        Vector2 bulletCenter = {
            bullet->rect.x + bullet->rect.width / 2.0f,
            bullet->rect.y + bullet->rect.height / 2.0f
        };
        float bulletRadius = bullet->rect.width / 2.0f;

        for (int j = 0; j < ENEMY_COUNT; j++) {
            Enemy *enemy = &enemyManager->enemies[j];

            if (!enemy->active || enemy->isExploding) continue;

            float enemyRadius = ENEMY_SIZE / 2.0f;

            if (CheckCollisionCircles(bulletCenter, bulletRadius, enemy->position, enemyRadius)) {

                bullet->active = false;

                // Dano para inimigos normais
                int damage = 1;
                if (bullet->type == 2) damage = 2;
                if (bullet->type == 3) damage = 4;

                enemy->health -= damage;
                enemy->hitTimer = ENEMY_FLASH_DURATION;

                if (enemy->health <= 0) {
                    enemy->active = false;
                    enemy->isExploding = true;
                    enemy->explosionTimer = ENEMY_EXPLOSION_DURATION;
                    enemyManager->activeCount--;

                    *playerGold += 2 + (enemy->type * 2);

                    PlaySound(audioManager->sfxExplosionEnemy);

                    ExplodeEnemy(enemyManager, enemy->position, EXPLOSION_PARTICLE_COUNT);

                    if (!enemyManager->bossActive && enemyManager->activeCount == 0) {
                         enemyManager->wavesCompletedCount++;
                         enemyManager->triggerShopReturn = true;
                    }

                } else {
                    PlaySound(audioManager->sfxWeak);
                }

                break;
            }
        }
    }
}