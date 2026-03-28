#ifndef SPAWN_H
#define SPAWN_H

#include "raylib.h"
#include "Enemy.h"
#include "Map.h"

// Time-based enemy spawn manager with map safety checks.
void UpdateSpawner(Enemy enemypool[], int emycpacity, Vector2 plpos, float* spawnTimer, float spawnRate, GameMap map);

#endif