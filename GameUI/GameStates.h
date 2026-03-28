#ifndef GAME_STATES_H
#define GAME_STATES_H

// High-level application states used by main loop update/draw switches.
typedef enum {
    STATE_TITLE,
    STATE_NAME_ENTRY,
    STATE_SETTINGS,
    STATE_GAMEPLAY,
    STATE_SCENE
} GameState;

#endif