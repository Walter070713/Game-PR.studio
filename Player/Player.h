#ifndef PLAYER_H
#define PLAYER_H
#include "raylib.h"
#include "Weapon.h"

// Player runtime data shared by movement/combat/UI systems.
typedef struct Player{
    Vector2 pos;
    Vector2 prevpos;
    Vector2 dir;
    char* name;
    float speed;
    float body;
    int health;
    int shield;
    int maxshield;
    float hurtTimer; // For I-Frames
    float shieldRegenTimer; // Time since last hit
    float shieldRegenAccum; // Accumulator for 1-second regen ticks
    Weapon weapon;  // Current equipped weapon
}Player;

// Initialize player state and default weapon.
void InitPlayer(Player* pl, Vector2 initpos);

// Apply WASD input and move player in world space.
void UpdatePlayerPos(Player* pl);

// Draw player body and name label.
void DrawPlayer(Player* pl);

#endif