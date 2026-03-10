#ifndef ENEMY_H
#define ENEMY_h
#include "raylib.h"
typedef struct Enemy{
    Vector2 pos;
    Vector2 dir;
    Color state;
    float body;
    int life;
    int damage;
}Enemy;
#endif