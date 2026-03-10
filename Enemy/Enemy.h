#ifndef ENEMY_H
#define ENEMY_H
#include "raylib.h"
#include "raymath.h"
#include "Player.h"
typedef struct Enemy{
    Vector2 pos;
    Vector2 dir;
    Color state;
    float body;
    float speed;
    int life;
    int damage;
    float flashtime;
}Enemy;
void InitEnemy(Enemy* enemy);
void UpdateEnemyPos(Enemy* enemy,Player* pl);
#endif