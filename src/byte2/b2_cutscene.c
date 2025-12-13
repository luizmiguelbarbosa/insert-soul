#include "b2_cutscene.h"
#include "raylib.h"
#include "raymath.h"
#include "b2_game_state.h"
#include "b2_audio.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// --- CONSTANTES DE ESTILO E ANIMAÇÃO ---
#define TITLE_FONT_SIZE 70
#define INSTRUCTION_FONT_SIZE 30
#define NEON_COLOR_BASE (Color){ 255, 0, 255, 255 }
#define TITLE_GLOW_COLOR (Color){ 0, 150, 255, 255 }

#define PLANET_CYCLE_DURATION 15.0f
#define EXPLOSION_START_TIME 10.0f
#define EXPLOSION_END_TIME 12.0f

// --- DECLARAÇÕES DAS FUNÇÕES AUXILIARES ---
static void DrawParallaxBackground(int screenWidth, int screenHeight, float time);
static void DrawNeonText(const char *text, int posX, int posY, int fontSize, float pulseSpeed, Color glowAura);

// --- FUNÇÃO DE INICIALIZAÇÃO DA INTRO ---
void InitCutscene(CutsceneScene *cs) {
    cs->isEnding = false;

    cs->pages[0]  = (CutscenePage){ "BYTE IN SPACE 2", 0.0f };
    cs->pages[1]  = (CutscenePage){ "Pressione ENTER para comecar", 0.0f };

    cs->currentPage = 0;
    cs->currentTimer = 0.0f;
    cs->textTimer = 0.0f;
    cs->visibleChars = 0;
    cs->isFadingOut = false;
    cs->titleAlpha = 0.0f;
    cs->showTitle = false;
}

// --- FUNÇÃO DE INICIALIZAÇÃO DO FINAL ---
void InitEnding(CutsceneScene *cs) {
    cs->isEnding = true;

    // 1. SEGURANÇA: Limpa o array de texturas para evitar "lixo" de memória
    // Isso evita o bug das "linhas" se a imagem não carregar.
    for (int i = 0; i < 5; i++) {
        cs->endingImages[i].id = 0;
        cs->endingImages[i].width = 0;
        cs->endingImages[i].height = 0;
    }

    // 2. Carrega sprites (Caminhos corrigidos e verificados)
    if (FileExists("assets/byte2/images/sprites/1.png")) cs->endingImages[0] = LoadTexture("assets/byte2/images/sprites/1.png");
    if (FileExists("assets/byte2/images/sprites/2.png")) cs->endingImages[1] = LoadTexture("assets/byte2/images/sprites/2.png");
    if (FileExists("assets/byte2/images/sprites/3.png")) cs->endingImages[2] = LoadTexture("assets/byte2/images/sprites/3.png");
    if (FileExists("assets/byte2/images/sprites/4.png")) cs->endingImages[3] = LoadTexture("assets/byte2/images/sprites/4.png");

    // CORREÇÃO: Havia um erro de digitação aqui ("byte2/byte2")
    if (FileExists("assets/byte2/images/sprites/5.png")) cs->endingImages[4] = LoadTexture("assets/byte2/images/sprites/5.png");

    cs->endingImageIndex = 0;

    PlayMusicTrack(&b2AudioManager, MUSIC_ENDING);
}

// --- FUNÇÃO DE UPDATE ---
void UpdateCutscene(CutsceneScene *cs, GameState *state, float deltaTime) {

    // >>> LÓGICA DO FINAL (QUADRINHOS) <<<
    if (cs->isEnding) {
        if (IsKeyPressed(KEY_Z)) {
            cs->endingImageIndex++;

            // Se passar da última imagem
            if (cs->endingImageIndex > 4) {
                // Descarrega texturas
                for(int i=0; i<5; i++) {
                    if (cs->endingImages[i].id > 0) UnloadTexture(cs->endingImages[i]);
                }

                StopMusicStream(b2AudioManager.musicEnding);
                cs->finished = true;
            }
        }
        return;
    }

    // >>> LÓGICA DA INTRO <<<
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        *state = STATE_SHOP;
        return;
    }

    if (cs->currentPage == 0) {
        cs->currentTimer += deltaTime;
        if (cs->currentTimer >= 3.0f) {
            cs->currentPage = 1;
        }
    }
}

// --- FUNÇÃO DE DRAW ---
void DrawCutscene(CutsceneScene *cs, int screenWidth, int screenHeight) {

    // --- DESENHO DO FINAL ---
    if (cs->isEnding) {
        ClearBackground(BLACK);

        const int COLUMNS = 2;
        const int ROWS = 3; // 2 linhas normais + 1 linha final larga
        const int MARGIN = 20; // Aumentei a margem um pouco

        float totalUsableWidth = (float)screenWidth - (COLUMNS + 1) * MARGIN;
        float totalUsableHeight = (float)screenHeight - (ROWS + 1) * MARGIN;

        // Tamanho máximo do "quadro" onde a imagem pode ficar
        float panelWidth = totalUsableWidth / COLUMNS;
        float panelHeight = totalUsableHeight / ROWS;

        for (int i = 0; i <= cs->endingImageIndex && i < 5; i++) {
            // Verifica se a textura é válida (ID > 0 e dimensões válidas)
            if (cs->endingImages[i].id <= 0 || cs->endingImages[i].width == 0) continue;

            Texture2D tex = cs->endingImages[i];

            int row = i / COLUMNS;
            int col = i % COLUMNS;

            // Define a área do painel (O "quadradinho" da HQ)
            Rectangle panelRect;

            if (i == 4) {
                // Última imagem ocupa a largura total embaixo
                panelRect.x = (float)MARGIN;
                panelRect.y = (float)MARGIN + (panelHeight + MARGIN) * 2;
                panelRect.width = (float)screenWidth - 2 * MARGIN;
                panelRect.height = panelHeight;
            } else {
                panelRect.x = (float)MARGIN + (panelWidth + MARGIN) * col;
                panelRect.y = (float)MARGIN + (panelHeight + MARGIN) * row;
                panelRect.width = panelWidth;
                panelRect.height = panelHeight;
            }

            // --- DESENHA A BORDA DO QUADRINHO ---
            DrawRectangleLinesEx(panelRect, 2.0f, WHITE);

            // --- CÁLCULO DE PROPORÇÃO (ASPECT RATIO) ---
            // Isso impede que a imagem fique esticada/esmagada
            float scaleX = panelRect.width / (float)tex.width;
            float scaleY = panelRect.height / (float)tex.height;
            float scale = (scaleX < scaleY) ? scaleX : scaleY; // Escolhe o menor para caber ("fit")

            float drawW = (float)tex.width * scale;
            float drawH = (float)tex.height * scale;

            // Centraliza a imagem dentro do painel
            float finalX = panelRect.x + (panelRect.width - drawW) / 2.0f;
            float finalY = panelRect.y + (panelRect.height - drawH) / 2.0f;

            // Desenha a textura com as proporções corretas
            DrawTexturePro(
                tex,
                (Rectangle){ 0, 0, (float)tex.width, (float)tex.height },
                (Rectangle){ finalX, finalY, drawW, drawH },
                (Vector2){ 0, 0 },
                0.0f,
                WHITE
            );
        }

        DrawText("Pressione [Z] para avancar...", screenWidth - 300, screenHeight - 30, 20, RAYWHITE);
        return;
    }

    // --- DESENHO DA INTRO (MANTIDO IGUAL) ---
    DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);
    DrawParallaxBackground(screenWidth, screenHeight, GetTime());

    const char *titleText = cs->pages[0].text;
    const char *instructionText = cs->pages[1].text;

    int titleWidth = MeasureText(titleText, TITLE_FONT_SIZE);
    int titlePosX = screenWidth / 2 - titleWidth / 2;
    int titlePosY = screenHeight / 2 - TITLE_FONT_SIZE / 2 - 50;

    DrawNeonText(titleText, titlePosX, titlePosY, TITLE_FONT_SIZE, 2.0f, TITLE_GLOW_COLOR);

    if (cs->currentPage == 1) {
        int instructionWidth = MeasureText(instructionText, INSTRUCTION_FONT_SIZE);
        int instructionPosX = screenWidth / 2 - instructionWidth / 2;
        int instructionPosY = screenHeight / 2 + 50;
        DrawNeonText(instructionText, instructionPosX, instructionPosY, INSTRUCTION_FONT_SIZE, 4.0f, NEON_COLOR_BASE);
    }
}

// --- FUNÇÕES AUXILIARES ---
static void DrawParallaxBackground(int screenWidth, int screenHeight, float time) {
    float cycleTime = fmodf(time, PLANET_CYCLE_DURATION);

    // 1. Estrelas de fundo
    for (int i = 0; i < 200; i++) {
        int x = (i * 73) % screenWidth;
        int y = (i * 59) % screenHeight;
        float movementX = sin(time * 0.05f) * 10.0f;
        float movementY = cos(time * 0.03f) * 5.0f;
        int finalX = (x + (int)movementX) % screenWidth;
        int finalY = (y + (int)movementY) % screenHeight;
        if (finalX < 0) finalX += screenWidth;
        if (finalY < 0) finalY += screenHeight;
        DrawCircle(finalX, finalY, 1, (Color){ 150, 100, 100, 100 });
    }

    // 2. Estrelas médias
    for (int i = 0; i < 100; i++) {
        int x = (i * 97) % screenWidth;
        int y = (i * 83) % screenHeight;
        float movementX = cos(time * 0.15f) * 20.0f;
        float movementY = sin(time * 0.10f) * 15.0f;
        int finalX = (x + (int)movementX) % screenWidth;
        int finalY = (y + (int)movementY) % screenHeight;
        if (finalX < 0) finalX += screenWidth;
        if (finalY < 0) finalY += screenHeight;
        DrawCircle(finalX, finalY, (i % 2) + 1, (Color){ 150, 150, 200, 150 });
    }

    // 3. Estrelas rápidas
    for (int i = 0; i < 50; i++) {
        int x = (i * 121) % screenWidth;
        int y = (i * 107) % screenHeight;
        float movementX = sin(time * 0.25f) * 30.0f;
        float movementY = cos(time * 0.20f) * 25.0f;
        int finalX = (x + (int)movementX) % screenWidth;
        int finalY = (y + (int)movementY) % screenHeight;
        if (finalX < 0) finalX += screenWidth;
        if (finalY < 0) finalY += screenHeight;
        DrawCircle(finalX, finalY, 1, (Color){ 255, 255, 255, 200 });
    }

    // 4. Planeta
    Vector2 planetPos = { (float)screenWidth * 0.7f, (float)screenHeight * 0.3f };
    float planetBaseSize = 80.0f;
    Color planetBaseColor = { 50, 50, 100, 255 };
    float currentPlanetSize = planetBaseSize;
    float atmosphereAlpha = 1.0f;

    if (cycleTime >= EXPLOSION_START_TIME && cycleTime < EXPLOSION_END_TIME) {
        float explosionPhase = (cycleTime - EXPLOSION_START_TIME) / (EXPLOSION_END_TIME - EXPLOSION_START_TIME);
        float blastRadius = planetBaseSize * (1.0f + explosionPhase * 2.0f);
        DrawCircleV(planetPos, blastRadius, Fade(RED, 1.0f - explosionPhase));
        currentPlanetSize = planetBaseSize * (1.0f - explosionPhase);
        atmosphereAlpha = 1.0f - explosionPhase;
    }
    else if (cycleTime >= EXPLOSION_END_TIME) {
        float reformPhase = (cycleTime - EXPLOSION_END_TIME) / (PLANET_CYCLE_DURATION - EXPLOSION_END_TIME);
        currentPlanetSize = planetBaseSize * reformPhase;
        float pulse = (sin(time * 8.0f) + 1.0f) / 2.0f;
        DrawCircleV(planetPos, planetBaseSize * 1.5f, Fade(TITLE_GLOW_COLOR, 0.4f * pulse * (1.0f - reformPhase)));
        atmosphereAlpha = reformPhase;
    }

    if (currentPlanetSize > 1.0f) {
        Color atmosphereColor = { 100, 100, 150, (unsigned char)(120 * atmosphereAlpha) };
        DrawCircleV(planetPos, currentPlanetSize * 1.3f, Fade(atmosphereColor, 0.5f));
        DrawCircleV(planetPos, currentPlanetSize * 1.1f, Fade(atmosphereColor, 0.8f * atmosphereAlpha));
        DrawCircleV(planetPos, currentPlanetSize, Fade(planetBaseColor, atmosphereAlpha));
    }

    // 5. Cometas
    Color colors[] = { RED, GREEN, YELLOW, TITLE_GLOW_COLOR, NEON_COLOR_BASE, ORANGE, LIME, VIOLET };
    for (int i = 0; i < 10; i++) {
        float speed = 250.0f + (i * 20.0f);
        float totalMovement = fmodf(time * speed, (float)screenWidth * 1.5f);
        float startOffset = (float)(i * 150);
        float xPos = (float)screenWidth + 100.0f - totalMovement + startOffset * 0.2f;
        float yPos = 0.0f - 50.0f + totalMovement * 0.7f + startOffset * 0.4f;

        if (xPos < -100.0f || yPos > (float)screenHeight + 100.0f) {
            xPos += (float)screenWidth * 2.0f + 200.0f;
            yPos -= (float)screenHeight * 1.5f;
        }

        DrawLineEx((Vector2){xPos, yPos}, (Vector2){ xPos + 20, yPos - 15 }, 3.0f, Fade(colors[i % 8], 0.7f));
        DrawCircleV((Vector2){xPos, yPos}, 3, Fade(colors[i % 8], 1.0f));
    }
}

static void DrawNeonText(const char *text, int posX, int posY, int fontSize, float pulseSpeed, Color glowAura) {
    float pulse = 1.0f;
    if (pulseSpeed > 0) pulse = (sin(GetTime() * pulseSpeed) + 1.0f) / 2.0f;
    Color glowColor = glowAura;
    glowColor.a = (unsigned char)(glowAura.a * (0.3f + pulse * 0.5f));

    DrawText(text, posX - 4, posY, fontSize, glowColor);
    DrawText(text, posX + 4, posY, fontSize, glowColor);
    DrawText(text, posX, posY - 4, fontSize, glowColor);
    DrawText(text, posX, posY + 4, fontSize, glowColor);
    DrawText(text, posX - 2, posY - 2, fontSize, glowColor);
    DrawText(text, posX + 2, posY + 2, fontSize, glowColor);
    DrawText(text, posX - 2, posY + 2, fontSize, glowColor);
    DrawText(text, posX + 2, posY - 2, fontSize, glowColor);
    DrawText(text, posX, posY, fontSize, NEON_COLOR_BASE);
}