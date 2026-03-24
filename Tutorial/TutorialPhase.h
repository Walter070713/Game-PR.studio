#ifndef TUTORIAL_PHASE_H
#define TUTORIAL_PHASE_H

#include <stdbool.h>
#include "raylib.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"

typedef struct {
    bool isActive;
    bool enemiesSpawned;
    float spawnDelayTimer;
    float hintTimer;
    int currentWave;
    int totalWaves;
} TutorialFlow;

void InitTutorialFlow(TutorialFlow* flow);
void StartTutorialFlow(TutorialFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer);
bool UpdateTutorialFlow(TutorialFlow* flow, Player* player, GameMap* room,
    Enemy enemyPool[], int enemyCapacity, Bullet bulletPool[], int bulletCapacity, Camera2D camera);

void DrawTutorialWorldOverlay(const TutorialFlow* flow, const Player* player, Enemy enemyPool[], int enemyCapacity,
    Bullet bulletPool[], int bulletCapacity, Camera2D camera);
void DrawTutorialHUD(const TutorialFlow* flow, const Player* player);

#endif