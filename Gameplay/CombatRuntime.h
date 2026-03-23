#ifndef COMBAT_RUNTIME_H
#define COMBAT_RUNTIME_H

#include "raylib.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "MouseAim.h"
#include "Map.h"

void UpdateCombatRuntime(Player* player, Enemy enemyPool[], int enemyCapacity,
    Bullet bulletPool[], int bulletCapacity, MseAim* mouse, Camera2D camera,
    GameMap room, float* spawnTimer, float spawnRate);

void DrawCombatRuntimeWorld(Player* player, Enemy enemyPool[], int enemyCapacity,
    Bullet bulletPool[], int bulletCapacity, MseAim* mouse);

void DrawCombatRuntimeHUD(Player* player);

void ResetCombatRuntime(Enemy enemyPool[], int enemyCapacity,
    Bullet bulletPool[], int bulletCapacity, float* spawnTimer);

#endif