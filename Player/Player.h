#ifndef PLAYER_H
#define PLAYER_H
#include "raylib.h"
#include "Weapon.h"

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

void InitPlayer(Player* pl, Vector2 initpos);
void UpdatePlayerPos(Player* pl);
void DrawPlayer(Player* pl);

#endif