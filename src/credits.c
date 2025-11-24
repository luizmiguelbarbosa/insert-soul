#include <raylib.h>
#include <credits.h>
#include <math.h>

void ShowCredits(void){
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    const char *credits[] =  {
"INSERT YOUR SOUX",
"",
"=== ABOUT THIS PROJECT ===",
"This game was developed as a college project.",
"A collaborative work created by students,",
"where everyone contributed together to",
"programming, design, art, mechanics and ideas.",
"",
"=== TEAM ===",
"Rafael M.",
"Gustavo",
"Miguel",
"Miqueis",
"Egilson Francisco (Special Music Contribution)",
"",
"=== MUSIC & SOUND ===",
"Original Soundtrack: Raylib OST",
"Additional Music: Egilson Francisco",
"Sound Effects: Team Collaboration",
"",
"=== SPECIAL THANKS ===",
"To classmates, friends and family,",
"for supporting the development of this project.",
"",
"© 2025 — College Project: INSERT YOUR SOUX",
"",
"Thank you for playing!",
"",
"Press ENTER to return"


    };

    int lineCount = sizeof(credits) / sizeof(credits[0]);
    float scrollY = screenHeight;
    float speed = 70.0f; // Velocidade da rolagem

    while (!WindowShouldClose())
{
    // Atualiza a rolagem
    scrollY -= GetFrameTime() * speed;
    if (scrollY < -lineCount * 30) scrollY = screenHeight;  // repete a rolagem

    // Atualiza timer para o piscar do texto
    static float blinkTimer = 0.0f;
    blinkTimer += GetFrameTime();
    float alpha = 0.5f + 0.5f * sinf(blinkTimer * 3.0f); // varia entre 0 e 1

    BeginDrawing();
    ClearBackground(BLACK);

    // Desenha as linhas dos créditos centralizadas
    for (int i = 0; i < lineCount; i++) {
        DrawText(credits[i],
                 screenWidth/2 - MeasureText(credits[i], 20)/2,
                 scrollY + i * 30,
                 20,
                 RAYWHITE);
    }

    // Texto piscando no canto inferior direito
    DrawText("Press ENTER to return",
             screenWidth - MeasureText("Press ENTER to return", 20) - 20,
             screenHeight - 40,
             20,
             (Color){GRAY.r, GRAY.g, GRAY.b, (unsigned char)(255 * alpha)});

    EndDrawing();

    // Sai da tela de créditos ao apertar ENTER
    if (IsKeyPressed(KEY_ENTER))
        break;
}

}
