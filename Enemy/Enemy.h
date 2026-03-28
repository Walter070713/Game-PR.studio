#ifndef ENEMY_H
#define ENEMY_H
#include "raylib.h"
#include "raymath.h"
#include "Player.h"
typedef struct Bullet Bullet;

// Runtime enemy unit used by both combat and tutorial flows.
typedef struct Enemy{
    Vector2 pos;
    Vector2 prevpos;
    Color state;
    float body;
    float speed;
    int health;
    bool active;
    float flashtime;
    const char* name;
}Enemy;

// Fill enemy pool with default active enemies.
void InitEnemy(Enemy enemypool[], int emycapacity);

// Move active enemies toward player and update hit-flash timer.
void UpdateEnemyHorde(Enemy enemypool[], int emycapacity, Vector2 plpos);

// Draw active enemies and optional name labels.
void DrawEnemy(Enemy enemypool[], int emycapacity);

#endif