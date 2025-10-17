#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include "raylib.h"
#include <stdbool.h>

typedef struct VideoPlayer {
    int frameCount;
    int currentFrame;
    float frameTime;
    float timer;

    Music music;
    bool audioPlayed;
} VideoPlayer;

// Inicializa player (pré-carrega frames e áudio)
bool VideoPlayer_Init(VideoPlayer *vp, const char *framesPathFormat, int frameCount, float fps, const char *audioPath);

// Atualiza player (frames + áudio)
void VideoPlayer_Update(VideoPlayer *vp, float delta);

// Desenha frame atual
void VideoPlayer_Draw(VideoPlayer *vp, int x, int y, int width, int height);

// Libera recursos
void VideoPlayer_Unload(VideoPlayer *vp);

// Retorna true se terminou (último frame e áudio)
bool VideoPlayer_IsFinished(VideoPlayer *vp);

#endif // VIDEO_PLAYER_H
