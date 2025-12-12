#include "b2_audio.h"
#include "raylib.h"
#include <stddef.h> // Para NULL

// --- DEFINIÇÃO DA VARIÁVEL GLOBAL ---
// É aqui que a variável é criada na memória.
// O "extern" no .h apenas aponta para cá.
AudioManager b2AudioManager = { 0 };

// --- IMPLEMENTAÇÃO DAS FUNÇÕES ---

void InitAudioManager(AudioManager *manager) {
    if (manager == NULL) return;

    // Inicializa Músicas (ajuste caminhos conforme necessário)
    // Dica: Use FileExists para evitar crash se o arquivo não existir
    if (FileExists("assets/byte2/audio/music_shop.ogg")) manager->musicShop = LoadMusicStream("assets/byte2/audio/music_shop.ogg");
    if (FileExists("assets/byte2/audio/music_gameplay.ogg")) manager->musicGameplay = LoadMusicStream("assets/byte2/audio/music_gameplay.ogg");
    if (FileExists("assets/byte2/audio/music_cutscene.ogg")) manager->musicCutscene = LoadMusicStream("assets/byte2/audio/music_cutscene.ogg");
    if (FileExists("assets/byte2/audio/music_ending.ogg")) manager->musicEnding = LoadMusicStream("assets/byte2/audio/music_ending.ogg");

    manager->currentMusic = NULL;

    // Inicializa SFX
    if (FileExists("assets/byte2/audio/sfx_weak.wav")) manager->sfxWeak = LoadSound("assets/byte2/audio/sfx_weak.wav");
    if (FileExists("assets/byte2/audio/sfx_medium.wav")) manager->sfxMedium = LoadSound("assets/byte2/audio/sfx_medium.wav");
    if (FileExists("assets/byte2/audio/sfx_strong.wav")) manager->sfxStrong = LoadSound("assets/byte2/audio/sfx_strong.wav");
    if (FileExists("assets/byte2/audio/sfx_charge.wav")) manager->sfxCharge = LoadSound("assets/byte2/audio/sfx_charge.wav");
    if (FileExists("assets/byte2/audio/sfx_explosion.wav")) manager->sfxExplosionEnemy = LoadSound("assets/byte2/audio/sfx_explosion.wav");
}

void UpdateAudioManager(AudioManager *manager) {
    if (manager && manager->currentMusic != NULL) {
        UpdateMusicStream(*(manager->currentMusic));
    }
}

void PlayMusicTrack(AudioManager *manager, MusicType type) {
    if (!manager) return;

    // Para a música anterior se houver
    if (manager->currentMusic != NULL) {
        StopMusicStream(*(manager->currentMusic));
    }

    switch (type) {
        case MUSIC_SHOP:     manager->currentMusic = &manager->musicShop; break;
        case MUSIC_GAMEPLAY: manager->currentMusic = &manager->musicGameplay; break;
        case MUSIC_CUTSCENE: manager->currentMusic = &manager->musicCutscene; break;
        case MUSIC_ENDING:   manager->currentMusic = &manager->musicEnding; break;
        default:             manager->currentMusic = NULL; break;
    }

    // Toca a nova
    if (manager->currentMusic != NULL) {
        PlayMusicStream(*(manager->currentMusic));
    }
}

void PlayAttackSfx(AudioManager *manager, int attackType) {
    if (!manager) return;

    switch (attackType) {
        case ATTACK_WEAK:   if(manager->sfxWeak.stream.buffer) PlaySound(manager->sfxWeak); break;
        case ATTACK_MEDIUM: if(manager->sfxMedium.stream.buffer) PlaySound(manager->sfxMedium); break;
        case ATTACK_STRONG: if(manager->sfxStrong.stream.buffer) PlaySound(manager->sfxStrong); break;
    }
}

void PlayEnemyExplosionSfx(AudioManager *manager) {
    if (manager && manager->sfxExplosionEnemy.stream.buffer) {
        PlaySound(manager->sfxExplosionEnemy);
    }
}

void UnloadAudioManager(AudioManager *manager) {
    if (!manager) return;

    // Descarrega Músicas
    if (manager->musicShop.stream.buffer) UnloadMusicStream(manager->musicShop);
    if (manager->musicGameplay.stream.buffer) UnloadMusicStream(manager->musicGameplay);
    if (manager->musicCutscene.stream.buffer) UnloadMusicStream(manager->musicCutscene);
    if (manager->musicEnding.stream.buffer) UnloadMusicStream(manager->musicEnding);

    // Descarrega SFX
    if (manager->sfxWeak.stream.buffer) UnloadSound(manager->sfxWeak);
    if (manager->sfxMedium.stream.buffer) UnloadSound(manager->sfxMedium);
    if (manager->sfxStrong.stream.buffer) UnloadSound(manager->sfxStrong);
    if (manager->sfxCharge.stream.buffer) UnloadSound(manager->sfxCharge);
    if (manager->sfxExplosionEnemy.stream.buffer) UnloadSound(manager->sfxExplosionEnemy);
}