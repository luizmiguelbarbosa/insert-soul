#include "intro.h"
#include <math.h>

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

    // Tela de carregamento com círculo girando no canto inferior direito
    float timer = 0.0f;          // Começa em zero
    float angle = 0.0f;
    int radius = 20;              // Aumenta o tamanho do círculo
    int numDots = 8;             // Mantém a proporção
    float speed = 360.0f;
    float dotSize = radius * 0.25f; // Ponto proporcional ao raio

    int centerX = width - 80;
    int centerY = height - 80;

    // Aumenta o tempo de carregamento
    float targetTime = loadingTime + 1.4f; // por exemplo, 3s a mais

    while (!WindowShouldClose() && timer < targetTime) {
        float delta = GetFrameTime();
        timer += delta;
        angle += speed * delta;

        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < numDots; i++) {
            float a = angle + i * (360.0f / numDots);
            float rad = a * (3.1415926f / 180.0f);
            int dotX = centerX + (int)(radius * cos(rad));
            int dotY = centerY + (int)(radius * sin(rad));
            DrawCircle(dotX, dotY, dotSize, WHITE);
        }

        EndDrawing();
    }

    ShowCursor();
    VideoPlayer_Unload(vp);
    return true;
}
