#ifndef ENEMY_H
#define ENEMY_h
#include "raylib.h"
#include "Circle.h"
typedef struct Enemy{
    Vector2 pos;
    Vector2 dir;
    Circle body;
    Color state;
    int life;
    int damage;
}Enemy;
#endif