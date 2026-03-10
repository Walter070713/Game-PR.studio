#ifndef PLAYER_H
#define PLAYER_H
#include "raylib.h"
#include "Circle.h"
typedef struct Player{
    Vector2 pos;
    Vector2 dir;
    Circle body;
    char* name;
    float speed;
    int health;
    int shield;
}Player;
void InitPlayer(Player* pl,Vector2 initpos);
void UpdatePlayerPos(Player* pl);
void DrawPlayer(Player* pl);
#endif