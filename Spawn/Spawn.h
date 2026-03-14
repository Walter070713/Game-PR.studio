#ifndef SPAWN_H
#define SPAWN_H
#include "raylib.h"
#include "raymath.h"
#include "Enemy.h"
#include "Map.h"
void UpdateSpawner(Enemy enemypool[], int emycpacity, Vector2 plpos, float* spawnTimer, float spawnRate, GameMap map);
#endif