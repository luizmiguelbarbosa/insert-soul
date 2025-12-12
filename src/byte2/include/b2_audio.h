#ifndef B2_AUDIO_H // MUDADO DE AUDIO_H PARA B2_AUDIO_H
#define B2_AUDIO_H

#include "raylib.h"

// --- CONSTANTES DE REFERÊNCIA ---
#define ATTACK_WEAK 1
#define ATTACK_MEDIUM 2
#define ATTACK_STRONG 3

// --- ENUMERAÇÃO DE MÚSICAS ---
typedef enum {
    MUSIC_SHOP,
    MUSIC_GAMEPLAY,
    MUSIC_CUTSCENE,
    MUSIC_ENDING
} MusicType;

// --- ESTRUTURA DE DADOS ---
typedef struct AudioManager {
    // Músicas (Stream)
    Music musicShop;
    Music musicGameplay;
    Music musicCutscene;
    Music musicEnding;
    Music* currentMusic;

    // Efeitos Sonoros (Sound)
    Sound sfxWeak;
    Sound sfxMedium;
    Sound sfxStrong;
    Sound sfxCharge;
    Sound sfxExplosionEnemy;
} AudioManager;

// --- DECLARAÇÕES DE FUNÇÕES ---
void InitAudioManager(AudioManager *manager);
void UpdateAudioManager(AudioManager *manager);
void PlayMusicTrack(AudioManager *manager, MusicType type);
void PlayAttackSfx(AudioManager *manager, int attackType);
void PlayEnemyExplosionSfx(AudioManager *manager);
void UnloadAudioManager(AudioManager *manager);

// !!! A LINHA ABAIXO É A CORREÇÃO CRUCIAL !!!
// Permite que cutscene.c e outros arquivos acessem a variável criada no byte2.c
extern AudioManager b2AudioManager;

#endif // B2_AUDIO_H