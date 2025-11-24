#include "video_player.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 2

static Texture2D buffer[BUFFER_SIZE];
static char **framePaths = NULL;

bool VideoPlayer_Init(VideoPlayer *vp, const char *framesPathFormat, int frameCount, float fps, const char *audioPath) {
    vp->frameCount = frameCount;
    vp->currentFrame = 0;
    vp->frameTime = 1.0f / fps;
    vp->timer = 0.0f;
    vp->audioPlayed = false;

    framePaths = (char **)malloc(sizeof(char *) * frameCount);
    for (int i = 0; i < frameCount; i++) {
        framePaths[i] = (char *)malloc(256);
        sprintf(framePaths[i], framesPathFormat, i + 1);
    }

    for (int i = 0; i < BUFFER_SIZE && i < frameCount; i++) {
        Image img = LoadImage(framePaths[i]);
        if (!img.data) {
            printf("Erro ao carregar frame %s\n", framePaths[i]);
            return false;
        }
        buffer[i] = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    if (audioPath) {
        vp->music = LoadMusicStream(audioPath);
    }

    return true;
}

void VideoPlayer_Update(VideoPlayer *vp, float delta) {
    vp->timer += delta;

    if (!vp->audioPlayed && vp->music.ctxData != NULL) {
        PlayMusicStream(vp->music);
        vp->audioPlayed = true;
    }

    if (vp->audioPlayed) {
        UpdateMusicStream(vp->music);
    }

    while (vp->timer >= vp->frameTime) {
        vp->timer -= vp->frameTime;

        int oldIndex = vp->currentFrame % BUFFER_SIZE;
        vp->currentFrame++;

        int nextFrame = vp->currentFrame + BUFFER_SIZE - 1;
        if (nextFrame < vp->frameCount) {
            Image img = LoadImage(framePaths[nextFrame]);
            if (img.data) {
                if (buffer[oldIndex].id != 0) UnloadTexture(buffer[oldIndex]);
                buffer[oldIndex] = LoadTextureFromImage(img);
                UnloadImage(img);
            }
        }

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
                   (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                   (Rectangle){(float)(x + offsetX), (float)(y + offsetY), (float)drawWidth, (float)drawHeight},
                   (Vector2){0, 0}, 0.0f, WHITE);
}

void VideoPlayer_Unload(VideoPlayer *vp) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer[i].id) UnloadTexture(buffer[i]);
    }
    for (int i = 0; i < vp->frameCount; i++) {
        free(framePaths[i]);
    }
    free(framePaths);

    if (vp->audioPlayed) StopMusicStream(vp->music);
    UnloadMusicStream(vp->music);
}

bool VideoPlayer_IsFinished(VideoPlayer *vp) {
    return vp->currentFrame >= vp->frameCount;
}

// --- Nova função para reiniciar o vídeo ---
void VideoPlayer_Reset(VideoPlayer *vp) {
    vp->currentFrame = 0;
    vp->timer = 0.0f;
    if (vp->audioPlayed) {
        StopMusicStream(vp->music);
        vp->audioPlayed = false;
    }
}
