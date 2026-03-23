#ifndef OPENING_PHASE_H
#define OPENING_PHASE_H

#include "raylib.h"
#include <stdbool.h>
#include "Scene.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"

typedef enum {
    OPENING_NOT_STARTED,
    OPENING_DIALOG,
    OPENING_BLOCK_DIALOG,
    OPENING_SMALL_ROOM,
    OPENING_BIG_ROOM
} OpeningPhase;

typedef struct {
    OpeningPhase phase;
    Rectangle door;
    Rectangle interactBlock;
    bool hasInteractedWithBlock;
    float reminderTimer;
} OpeningFlow;

void InitOpeningFlow(OpeningFlow* flow);
void OpeningStartMission(OpeningFlow* flow, Scene* scene);
bool UpdateOpeningPeacefulPhase(OpeningFlow* flow, Player* player, GameMap* room,
    Enemy enemyPool[], int enemyCapacity, Bullet bulletPool[], float* spawnTimer, Scene* scene,
    bool* shouldEnterScene);
void OpeningHandleSceneComplete(OpeningFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer);

bool OpeningIsPeacefulPhase(const OpeningFlow* flow);
bool OpeningIsCombatEnabled(const OpeningFlow* flow);
bool OpeningIsComplete(const OpeningFlow* flow);
void DrawOpeningWorldOverlay(const OpeningFlow* flow);
void DrawOpeningHUD(const OpeningFlow* flow, const Player* player);

#endif