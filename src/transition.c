#include "include/transition.h"
#include <stdlib.h>
#include <string.h>


// O módulo está incompleto( fase de teste)
#define DISSOLVE_MAP_SIZE 256

// Função para criar um mapa aleatório para dissolve
static void GenerateDissolveMap(unsigned char *map, int size) {
    for (int i = 0; i < size; i++) {
        map[i] = rand() % 256;
    }
}

void Transition_Init(Transition *t, int width, int height, float speed, TransitionType type) {
    t->width = width;
    t->height = height;
    t->speed = speed;
    t->progress = 0.0f;
    t->active = true;
    t->type = type;

    t->dissolveMap = NULL;

    // Carrega os shaders se necessário (para blur, dissolve, etc)
    t->shader = LoadShader(0, 0); // Nenhum shader por padrão

    if (type == TRANSITION_DISSOLVE) {
        t->dissolveMap = (unsigned char *)malloc(DISSOLVE_MAP_SIZE);
        GenerateDissolveMap(t->dissolveMap, DISSOLVE_MAP_SIZE);
    }

    if (type == TRANSITION_BLUR) {
        // Você pode carregar aqui o shader de blur
        // Exemplo: t->shader = LoadShader(0, "shaders/blur.fs");
    }
}

bool Transition_UpdateDraw(Transition *t, Texture2D *from, Texture2D *to) {
    if (!t->active) return false;

    t->progress += t->speed;
    if (t->progress > 1.0f) t->progress = 1.0f;

    BeginDrawing();
    ClearBackground(BLACK);

    switch (t->type) {
        case TRANSITION_FADE:
            // Fade out de 'from' para 'to'
            DrawTexture(*from, 0, 0, WHITE);
            DrawTexture(*to, 0, 0, (Color){255, 255, 255, (unsigned char)(255 * t->progress)});
            break;

        case TRANSITION_ZOOM: {
            // Zoom in de 'from' e zoom out de 'to'
            float scaleFrom = 1.0f - t->progress;
            float scaleTo = t->progress;

            int cx = t->width / 2;
            int cy = t->height / 2;

            Vector2 origin = { (float)(from->width / 2), (float)(from->height / 2) };

            DrawTexturePro(*from,
                (Rectangle){0, 0, (float)from->width, (float)from->height},
                (Rectangle){cx, cy, from->width * scaleFrom, from->height * scaleFrom},
                origin, 0.0f, Fade(WHITE, 1.0f - t->progress));

            DrawTexturePro(*to,
                (Rectangle){0, 0, (float)to->width, (float)to->height},
                (Rectangle){cx, cy, to->width * scaleTo, to->height * scaleTo},
                origin, 0.0f, Fade(WHITE, t->progress));
            break;
        }

        case TRANSITION_DISSOLVE:
            // Desenhar 'from' normalmente
            DrawTexture(*from, 0, 0, WHITE);
            // Desenhar 'to' com dissolução (simplificada)
            DrawTexture(*to, 0, 0, (Color){255, 255, 255, (unsigned char)(255 * t->progress)});
            break;

        case TRANSITION_WIPE: {
            // Wipe horizontal de 'from' para 'to'
            int wipeWidth = (int)(t->width * t->progress);
            DrawTexture(*from, 0, 0, WHITE);
            DrawTextureRec(*to, (Rectangle){0, 0, (float)wipeWidth, (float)t->height}, (Vector2){0, 0}, WHITE);
            break;
        }

        case TRANSITION_BLINK: {
            // Piscar entre as duas imagens
            int blinkInterval = 10; // frames por "piscar"
            int frame = (int)(t->progress * 60 * 5); // total ~ 5 segundos
            if ((frame / blinkInterval) % 2 == 0) {
                DrawTexture(*from, 0, 0, WHITE);
            } else {
                DrawTexture(*to, 0, 0, WHITE);
            }
            break;
        }

        case TRANSITION_BLUR:
            // Apenas desenha 'to' para simplificação (sem shader de blur)
            DrawTexture(*to, 0, 0, Fade(WHITE, t->progress));
            break;

        default:
            DrawTexture(*to, 0, 0, WHITE);
            break;
    }

    EndDrawing();

    if (t->progress >= 1.0f) {
        t->active = false;
        return false; // Transição terminada
    }

    return true; // Transição ainda em andamento
}

void Transition_Unload(Transition *t) {
    if (t->dissolveMap) {
        free(t->dissolveMap);
        t->dissolveMap = NULL;
    }
    if (t->shader.id != 0) {
        UnloadShader(t->shader);
        t->shader.id = 0;
    }
}
