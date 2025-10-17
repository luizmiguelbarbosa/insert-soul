#ifndef INTRO_H
#define INTRO_H

#include "video_player.h"
#include "raylib.h"
#include <stdbool.h>

bool Intro_Play(VideoPlayer *vp, int width, int height, const char *framesPath, int frameCount, float fps, const char *audioPath, float loadingTime);

#endif // INTRO_H
