#ifndef GAME_STATE_H
#define GAME_STATE_H // Início do header guard

/**
 * @brief Enumeração que define os diferentes estados ou telas do jogo.
 * O jogo sempre estará em um destes estados em qualquer momento.
 */
typedef enum GameState {
    STATE_TITLE = 0,    // Tela de Título (Intro Cutscene)
    STATE_GAMEPLAY,     // Fase principal de ação/tiro (Shoot 'em Up)
    STATE_SHOP,         // Tela da Loja (onde o jogador compra upgrades)
    STATE_GAME_OVER,    // Tela de Fim de Jogo (perda de todas as vidas/game over)
    STATE_CUTSCENE,     // Um estado genérico de cutscene (pode ser usado para intros longas ou transições)
    STATE_ENDING        // Tela final do jogo, após a vitória.
} GameState;

#endif // GAME_STATE_H