#include "video_player.h"
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 2

static Texture2D buffer[BUFFER_SIZE];
static char **framePaths;

bool VideoPlayer_Init(VideoPlayer *vp, const char *framesPathFormat, int frameCount, float fps, const char *audioPath) {
    vp->frameCount = frameCount;
    vp->currentFrame = 0;
    vp->frameTime = 1.0f / fps;
    vp->timer = 0.0f;
    vp->audioPlayed = false;

    // Aloca caminhos para todos os frames
    framePaths = (char**)malloc(sizeof(char*) * frameCount);
    for (int i = 0; i < frameCount; i++) {
        framePaths[i] = (char*)malloc(256);
        sprintf(framePaths[i], framesPathFormat, i + 1);
    }

    // Pré-carrega os primeiros frames no buffer
    for (int i = 0; i < BUFFER_SIZE && i < frameCount; i++) {
        Image img = LoadImage(framePaths[i]);
        if (!img.data) {
            printf("Erro ao carregar frame %s\n", framePaths[i]);
            return false;
        }
        buffer[i] = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    // Inicializa espaços restantes do buffer
    for (int i = BUFFER_SIZE; i < BUFFER_SIZE; i++) buffer[i] = (Texture2D){0};

    // Carrega áudio, se houver
    if (audioPath != NULL) {
        vp->music = LoadMusicStream(audioPath);
        vp->audioPlayed = false;
    }

    return true;
}

void VideoPlayer_Update(VideoPlayer *vp, float delta) {
    vp->timer += delta;

    // Inicia o áudio apenas uma vez
    if (!vp->audioPlayed && vp->music.ctxData != NULL) {
        PlayMusicStream(vp->music);
        vp->audioPlayed = true;
    }

    // Atualiza áudio se ainda tocando e vídeo não acabou
    if (vp->audioPlayed && vp->currentFrame < vp->frameCount) {
        UpdateMusicStream(vp->music);
    }

    // Avança frames
    while (vp->timer >= vp->frameTime) {
        vp->timer -= vp->frameTime;

        int oldIndex = vp->currentFrame % BUFFER_SIZE;
        vp->currentFrame++;

        // Pré-carrega próximo frame
        int nextFrame = vp->currentFrame + BUFFER_SIZE - 1;
        if (nextFrame < vp->frameCount) {
            Image img = LoadImage(framePaths[nextFrame]);
            if (!img.data) continue;
            if (buffer[oldIndex].id != 0) UnloadTexture(buffer[oldIndex]);
            buffer[oldIndex] = LoadTextureFromImage(img);
            UnloadImage(img);
        }

        // Para áudio no último frame
        if (vp->currentFrame >= vp->frameCount && vp->audioPlayed) {
            StopMusicStream(vp->music);
        }
    }
}

void VideoPlayer_Draw(VideoPlayer *vp, int x, int y, int width, int height) {
    if (vp->currentFrame >= vp->frameCount) return;

    int idx = vp->currentFrame % BUFFER_SIZE;
    Texture2D tex = buffer[idx];

    float scaleX = (float)width / tex.width;
    float scaleY = (float)height / tex.height;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    int drawWidth = tex.width * scale;
    int drawHeight = tex.height * scale;
    int offsetX = (width - drawWidth) / 2;
    int offsetY = (height - drawHeight) / 2;

    DrawTexturePro(tex,
                   (Rectangle){0,0,tex.width,tex.height},
                   (Rectangle){offsetX,offsetY,drawWidth,drawHeight},
                   (Vector2){0,0}, 0.0f, WHITE);
}

void VideoPlayer_Unload(VideoPlayer *vp) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer[i].id != 0) UnloadTexture(buffer[i]);
    }

    for (int i = 0; i < vp->frameCount; i++) free(framePaths[i]);
    free(framePaths);

    if (vp->audioPlayed) StopMusicStream(vp->music);
    UnloadMusicStream(vp->music);
}

bool VideoPlayer_IsFinished(VideoPlayer *vp) {
    // Termina quando último frame foi mostrado
    return vp->currentFrame >= vp->frameCount;
}
