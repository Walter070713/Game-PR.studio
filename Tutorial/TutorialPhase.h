#ifndef TUTORIAL_PHASE_H
#define TUTORIAL_PHASE_H

#include <stdbool.h>
#include "raylib.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"

// Runtime state for tutorial wave flow and UI hints.
typedef struct {
    bool isActive;
    bool enemiesSpawned;
    float spawnDelayTimer;
    float hintTimer;
    int currentWave;
    int totalWaves;
} TutorialFlow;

// Initialize tutorial flow to inactive defaults.
void InitTutorialFlow(TutorialFlow* flow);

// Switch to tutorial room and reset combat runtime for tutorial pacing.
void StartTutorialFlow(TutorialFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer);

// Update tutorial combat waves and completion checks.
bool UpdateTutorialFlow(TutorialFlow* flow, Player* player, GameMap* room,
    Enemy enemyPool[], int enemyCapacity, Bullet bulletPool[], int bulletCapacity, Camera2D camera);

// Draw tutorial-specific world overlays (enemies, bullets, aim line).
void DrawTutorialWorldOverlay(const Player* player, Enemy enemyPool[], int enemyCapacity,
    Bullet bulletPool[], int bulletCapacity, Camera2D camera);

// Draw tutorial-only HUD prompts.
void DrawTutorialHUD(const TutorialFlow* flow);

#endif