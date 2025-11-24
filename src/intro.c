#include <intro.h>
#include <math.h>
#include <video_player.h>
#include <raylib.h>

bool Intro_Play(VideoPlayer *vp, int width, int height, const char *framesPath, int frameCount, float fps, const char *audioPath, float loadingTime) {
    if (!VideoPlayer_Init(vp, framesPath, frameCount, fps, audioPath)) return false;

    HideCursor();

    // Loop do vídeo
    while (!WindowShouldClose() && !VideoPlayer_IsFinished(vp)) {
        float delta = GetFrameTime();
        VideoPlayer_Update(vp, delta);

        BeginDrawing();
        ClearBackground(BLACK);
        VideoPlayer_Draw(vp, 0, 0, width, height);
        EndDrawing();
    }

    ShowCursor();
    VideoPlayer_Unload(vp);
    return true;
}
