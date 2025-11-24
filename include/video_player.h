#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <raylib.h>
#include <stdbool.h>

typedef struct {
    int frameCount;
    int currentFrame;
    float frameTime;
    float timer;
    bool audioPlayed;
    Music music;
} VideoPlayer;

bool VideoPlayer_Init(VideoPlayer *vp, const char *framesPathFormat, int frameCount, float fps, const char *audioPath);
void VideoPlayer_Update(VideoPlayer *vp, float delta);
void VideoPlayer_Draw(VideoPlayer *vp, int x, int y, int width, int height);
void VideoPlayer_Unload(VideoPlayer *vp);
bool VideoPlayer_IsFinished(VideoPlayer *vp);
void VideoPlayer_Reset(VideoPlayer *vp); // ✅ adicionada
#endif
