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

// Opening chapter sub-phases handled inside gameplay/scene transitions.
typedef enum {
    OPENING_NOT_STARTED,
    OPENING_DIALOG,
    OPENING_RONDY_DIALOG,
    OPENING_MAP,
    OPENING_COMPLETE
} OpeningPhase;

// Runtime payload for opening chapter exploration and interactions.
typedef struct {
    OpeningPhase phase;
    Rectangle door;
    Rectangle transportZone;
    Vector2 transportPoint;
    float movementHintTimer;      // Timer for movement hint display
    bool playerHasMovedInOpening; // Tracks if player has moved this session
} OpeningFlow;

// Initialize opening flow state.
void InitOpeningFlow(OpeningFlow* flow);

// Start opening story scene and set opening chapter active.
void OpeningStartMission(OpeningFlow* flow, Scene* scene, const Player* player);

// Update opening exploration logic and emit transition flags.
bool UpdateOpeningPeacefulPhase(OpeningFlow* flow, Player* player, GameMap* room,
    Scene* scene, NPCPool* npcpool,
    bool* shouldEnterScene, bool* shouldStartTutorial);

// React to scene completion and advance opening phase.
void OpeningHandleSceneComplete(OpeningFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer, NPCPool* npcpool);

// Draw opening-only prompts and interaction hints.
void DrawOpeningWorldOverlay(const OpeningFlow* flow, const Player* player, const NPCPool* npcpool);

// Release opening-specific loaded scene data.
void ShutdownOpeningFlowAssets(void);

#endif