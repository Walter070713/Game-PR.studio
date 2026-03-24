#ifndef OPENING_PHASE_H
#define OPENING_PHASE_H

#include "raylib.h"
#include <stdbool.h>
#include "Scene.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "NPC.h"

typedef enum {
    OPENING_NOT_STARTED,
    OPENING_DIALOG,
    OPENING_RONDY_DIALOG,
    OPENING_MAP,
    OPENING_COMPLETE
} OpeningPhase;

typedef struct {
    OpeningPhase phase;
    Rectangle door;
    Rectangle transportZone;
    Vector2 transportPoint;
    float reminderTimer;
    float movementHintTimer;      // Timer for movement hint display
    bool playerHasMovedInOpening; // Tracks if player has moved this session
} OpeningFlow;

void InitOpeningFlow(OpeningFlow* flow);
void OpeningStartMission(OpeningFlow* flow, Scene* scene, GameMap* room, Player* player, 
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer, NPCPool* npcpool);
bool UpdateOpeningPeacefulPhase(OpeningFlow* flow, Player* player, GameMap* room,
    Enemy enemyPool[], int enemyCapacity, Bullet bulletPool[], float* spawnTimer, Scene* scene, NPCPool* npcpool,
    bool* shouldEnterScene, bool* shouldStartTutorial);
void OpeningHandleSceneComplete(OpeningFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer, NPCPool* npcpool);

bool OpeningIsPeacefulPhase(const OpeningFlow* flow);
bool OpeningIsCombatEnabled(const OpeningFlow* flow);
bool OpeningIsComplete(const OpeningFlow* flow);
void DrawOpeningWorldOverlay(const OpeningFlow* flow, const Player* player, const NPCPool* npcpool);
void DrawOpeningHUD(const OpeningFlow* flow, const Player* player);

#endif