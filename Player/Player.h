#ifndef PLAYER_H
#define PLAYER_H
#include "raylib.h"
typedef struct Player{
    Vector2 pos;
    Vector2 prevpos;
    Vector2 dir;
    char* name;
    float speed;
    float body;
    int health;
    int shield;
    
}Player;
void InitPlayer(Player* pl,Vector2 initpos);
void UpdatePlayerPos(Player* pl);
void DrawPlayer(Player* pl);
#endif