#ifndef ENEMY_H
#define ENEMY_H
#include "raylib.h"
typedef struct Enemy{
    Vector2 pos;
    Vector2 dir;
    Color state;
    float body;
    int life;
    int damage;
    float flashtime;
}Enemy;
void InitEnemy(Enemy* enemy);
#endif