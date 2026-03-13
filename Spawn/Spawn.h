#ifndef SPAWN_H
#define SPAWN_H
#include "raylib.h"
#include "raymath.h"
#include "Enemy.h"
void UpdateSpawner(Enemy enemypool[], int emycpacity, Vector2 playerPos, float* spawnTimer, float spawnRate);
#endif