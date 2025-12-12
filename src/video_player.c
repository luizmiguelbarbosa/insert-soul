#include "video_player.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 2
// Removida a constante MAX_LOAD_ATTEMPTS para simplificar o loop de timing,
// focando no tempo real (vp->frameTime).

static Texture2D buffer[BUFFER_SIZE];
static char **framePaths = NULL;

bool VideoPlayer_Init(VideoPlayer *vp, const char *framesPathFormat, int frameCount, float fps, const char *audioPath) {
    vp->frameCount = frameCount;
    vp->currentFrame = 0;
    vp->frameTime = 1.0f / fps;
    vp->timer = 0.0f;
    vp->audioPlayed = false;

    // Inicialização do Array de Caminhos
    framePaths = (char **)malloc(sizeof(char *) * frameCount);
    for (int i = 0; i < frameCount; i++) {
        framePaths[i] = (char *)malloc(256);
        sprintf(framePaths[i], framesPathFormat, i + 1);
    }

    // Pré-carregamento do Buffer Inicial
    for (int i = 0; i < BUFFER_SIZE && i < frameCount; i++) {
        // Inicializa o slot como vazio
        buffer[i] = (Texture2D){0};

        Image img = LoadImage(framePaths[i]);
        if (!img.data) {
            printf("Erro ao carregar frame inicial %s\n", framePaths[i]);
            // Limpa o que foi alocado antes de falhar
            VideoPlayer_Unload(vp);
            return false;
        }
        buffer[i] = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    // Configuração do Áudio
    vp->music = (Music){0};
    if (audioPath) {
        vp->music = LoadMusicStream(audioPath);
    }

    return true;
}

void VideoPlayer_Update(VideoPlayer *vp, float delta) {

    // Se o vídeo terminou, paramos de processar
    if (VideoPlayer_IsFinished(vp)) {
        if (vp->audioPlayed) StopMusicStream(vp->music);
        return;
    }

    vp->timer += delta;

    // Controle de Áudio
    if (!vp->audioPlayed && vp->music.ctxData != NULL) {
        PlayMusicStream(vp->music);
        vp->audioPlayed = true;
    }
    if (vp->audioPlayed) {
        UpdateMusicStream(vp->music);
    }

    // NOVO: Evita divisão por zero ou lógica errada se o FPS for zero
    if (vp->frameTime <= 0.0f) {
        return;
    }

    // Lógica de Troca de Frame e Buffering
    while (vp->timer >= vp->frameTime) {
        vp->timer -= vp->frameTime;

        int oldIndex = vp->currentFrame % BUFFER_SIZE;
        vp->currentFrame++;

        // Se o novo frame for o último frame do vídeo, paramos de carregar no buffer
        if (vp->currentFrame >= vp->frameCount) {
            if (vp->audioPlayed) StopMusicStream(vp->music);
            return;
        }

        // --- LÓGICA DE CARREGAMENTO (Background Loading) ---
        int nextFrameIndex = vp->currentFrame + BUFFER_SIZE - 1;

        if (nextFrameIndex < vp->frameCount) {

            // 1. Carregamento pesado do disco (I/O)
            Image img = LoadImage(framePaths[nextFrameIndex]);

            if (img.data) {
                // 2. Descarrega o slot antigo da VRAM
                // AQUI: Garantimos que o ID é válido antes de descarregar
                if (buffer[oldIndex].id != 0) UnloadTexture(buffer[oldIndex]);

                // 3. Carrega a nova textura para VRAM
                buffer[oldIndex] = LoadTextureFromImage(img);

                UnloadImage(img);
            } else {
                // Se o LoadImage falhar (disco lento/arquivo corrompido),
                // o slot de VRAM antigo fica sem ser atualizado.
                // O frame pode travar no último bom.
                // Log de erro:
                printf("AVISO: Falha na leitura do disco para o frame %d. Stuttering esperado.\n", nextFrameIndex);
            }
        }
    }
}
void VideoPlayer_Draw(VideoPlayer *vp, int x, int y, int width, int height) {
    if (VideoPlayer_IsFinished(vp)) return;

    int idx = vp->currentFrame % BUFFER_SIZE;
    Texture2D tex = buffer[idx];

    // Garante que a textura seja válida
    if (tex.id == 0) return;

    float scaleX = (float)width / tex.width;
    float scaleY = (float)height / tex.height;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    int drawWidth = tex.width * scale;
    int drawHeight = tex.height * scale;
    int offsetX = (width - drawWidth) / 2;
    int offsetY = (height - drawHeight) / 2;

    DrawTexturePro(tex,
                   (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                   (Rectangle){(float)(x + offsetX), (float)(y + offsetY), (float)drawWidth, (float)drawHeight},
                   (Vector2){0, 0}, 0.0f, WHITE);
}

void VideoPlayer_Unload(VideoPlayer *vp) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer[i].id) UnloadTexture(buffer[i]);
        buffer[i] = (Texture2D){0};
    }

    // Limpeza dos caminhos alocados
    if (framePaths) {
        for (int i = 0; i < vp->frameCount; i++) {
            if (framePaths[i]) free(framePaths[i]);
        }
        free(framePaths);
        framePaths = NULL;
    }

    if (vp->audioPlayed) StopMusicStream(vp->music);
    UnloadMusicStream(vp->music);
    vp->music = (Music){0};
    vp->audioPlayed = false;
}

bool VideoPlayer_IsFinished(VideoPlayer *vp) {
    return vp->currentFrame >= vp->frameCount;
}

void VideoPlayer_Reset(VideoPlayer *vp) {
    vp->currentFrame = 0;
    vp->timer = 0.0f;
    if (vp->audioPlayed) {
        StopMusicStream(vp->music);
        vp->audioPlayed = false;
    }
}