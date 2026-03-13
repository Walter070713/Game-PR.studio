#ifndef ENEMY_H
#define ENEMY_H
#include "raylib.h"
#include "raymath.h"
#include "window_setting.h"
#include "Player.h"
typedef struct Bullet Bullet;
typedef struct Enemy{
    Vector2 pos;
    Vector2 targetpos;
    Color state;
    float body;
    float speed;
    int health;
    bool active;
    int damage;
    float flashtime;
}Enemy;
void InitEnemy(Enemy enemypool[],int emycapacity);
void UpdateEnemyPos(Enemy* enemy,Player* pl);
void UpdateEnemyState(Enemy* enemy);
#endif