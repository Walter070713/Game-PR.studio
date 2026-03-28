#include "OpeningPhase.h"
#include "SceneData.h"
#include "Collision.h"
#include "CombatRuntime.h"

// Shared tuning values for opening interactions and map transport target.
static const float kDoorInteractPadding = 24.0f;
static const Vector2 kTutorialTransportPoint = {1215.0f, 1024.0f};

// Opening chapter scenes are loaded from external data files once and reused.
static SceneData gOpeningWakeNarrationScene = {0};
static SceneData gRondyOfficeIntroScene = {0};

// Load scene data on first use and keep it cached for subsequent entries.
static bool EnsureSceneLoaded(SceneData* scene, const char* sceneName, const char* sceneFile)
{
    if (!scene || !sceneName || !sceneFile) return false;
    if (scene->lines && scene->lineCount > 0) return true;
    return LoadSceneDataFromFile(sceneName, sceneFile, scene);
}

// If desired spawn point is blocked, search nearby ring positions for a walkable tile.
static Vector2 FindNearestWalkablePoint(Vector2 desired, Rectangle mapBounds)
{
    float tile = GetMapTileSize();
    if (tile < 1.0f) tile = 16.0f;

    if (!IsMapPointBlocked(desired, mapBounds)) return desired;

    for (int ring = 1; ring <= 10; ring++)
    {
        for (int oy = -ring; oy <= ring; oy++)
        {
            for (int ox = -ring; ox <= ring; ox++)
            {
                int ax = (ox < 0) ? -ox : ox;
                int ay = (oy < 0) ? -oy : oy;
                if (ax != ring && ay != ring) continue;

                Vector2 candidate = {
                    desired.x + (float)ox * tile * 0.5f,
                    desired.y + (float)oy * tile * 0.5f
                };

                if (candidate.x <= mapBounds.x || candidate.y <= mapBounds.y ||
                    candidate.x >= mapBounds.x + mapBounds.width ||
                    candidate.y >= mapBounds.y + mapBounds.height)
                {
                    continue;
                }

                if (!IsMapPointBlocked(candidate, mapBounds)) return candidate;
            }
        }
    }

    return desired;
}

// Helper for circular interaction checks against rectangular interactables.
static bool IsNearInteractable(const Player* player, Rectangle target)
{
    if (!player) return false;
    return CheckCollisionCircleRec(player->pos, player->body + kDoorInteractPadding, target);
}

// Query nearest interactable door zone around player for hint/prompts.
static bool IsDoorHintVisible(const Player* player, Rectangle mapBounds, Rectangle* outDoor)
{
    if (!player || !outDoor) return false;

    {
        float tile = GetMapTileSize();
        if (tile < 1.0f) tile = 16.0f;
        return FindNearbyDoorInteractZone(mapBounds, player->pos, tile * 7.0f, outDoor);
    }
}

// Find Rondy NPC close enough for interaction prompt/trigger.
static const NPC* FindNearbyRondy(const Player* player, const NPCPool* npcpool)
{
    if (!player || !npcpool) return NULL;

    for (int i = 0; i < npcpool->count; i++)
    {
        const NPC* npc = &npcpool->npcs[i];
        if (!npc->active || !npc->name || !TextIsEqual(npc->name, "Rondy")) continue;

        float interactRadius = player->body + npc->body * 1.8f;
        if (CheckCollisionCircles(player->pos, interactRadius, npc->pos, npc->body))
        {
            return npc;
        }
    }

    return NULL;
}

// Forward declaration
static void EnterOpeningMap(OpeningFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer, NPCPool* npcpool);

// Reset opening flow state before any chapter-specific logic runs.
void InitOpeningFlow(OpeningFlow* flow)
{
    if (!flow) return;

    // Default to inactive until Start Mission is triggered.
    flow->phase = OPENING_NOT_STARTED;
    flow->door = (Rectangle){0};
    flow->transportZone = (Rectangle){0};
    flow->transportPoint = kTutorialTransportPoint;
    flow->movementHintTimer = 5.0f;  // Show hint for 5 seconds initially
    flow->playerHasMovedInOpening = false;
}

// Start opening chapter by launching its first narrative scene.
void OpeningStartMission(OpeningFlow* flow, Scene* scene, const Player* player)
{
    if (!flow || !scene || !player) return;

    if (!EnsureSceneLoaded(&gOpeningWakeNarrationScene, "opening_wake_narration", SCENE_FILE_OPENING_WAKE)) return;

    SetActiveTMXMap("TEST MAP.tmx");

    InitScene(scene, &gOpeningWakeNarrationScene);
    flow->phase = OPENING_DIALOG;
}

// Internal setup for the first playable opening room.
static void EnterOpeningMap(OpeningFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer, NPCPool* npcpool)
{
    if (!flow || !room || !player || !bulletPool || !enemyPool || !spawnTimer || !npcpool) return;

    *room = InitRoom();

    flow->door = (Rectangle){0};
    {
        float zoneSize = GetMapTileSize() * 1.6f;
        flow->transportZone = (Rectangle){
            flow->transportPoint.x - zoneSize * 0.5f,
            flow->transportPoint.y - zoneSize * 0.5f,
            zoneSize,
            zoneSize
        };
    }

    // Spawn player in the down-left small room and ensure it is walkable.
    {
        float tile = GetMapTileSize();
        Vector2 playerDesired = {
            room->bounds.x + tile * 7.0f,
            room->bounds.y + tile * 15.0f
        };
        player->pos = FindNearestWalkablePoint(playerDesired, room->bounds);
    }
    player->prevpos = player->pos;

    ResetCombatRuntime(enemyPool, enemyCapacity, bulletPool, bulletPoolSize, spawnTimer);

    // Reset and spawn Rondy at the center of the whole big room.
    InitNPCPool(npcpool);
    {
        Vector2 rondyDesired = {
            room->bounds.x + room->bounds.width * 0.5f,
            room->bounds.y + room->bounds.height * 0.5f
        };
        Vector2 rondyPos = FindNearestWalkablePoint(rondyDesired, room->bounds);
        AddNPC(npcpool, rondyPos, "Rondy", player->body, WHITE);
    }

    flow->phase = OPENING_MAP;
}

// Drive opening chapter updates while player is in exploration mode.
bool UpdateOpeningPeacefulPhase(OpeningFlow* flow, Player* player, GameMap* room,
    Scene* scene, NPCPool* npcpool,
    bool* shouldEnterScene, bool* shouldStartTutorial)
{
    if (!flow || !player || !room || !scene || !npcpool || !shouldEnterScene || !shouldStartTutorial) return false;

    // Output flag tells main.c whether to switch from gameplay to scene mode.
    *shouldEnterScene = false;
    *shouldStartTutorial = false;

    if (flow->phase == OPENING_MAP || flow->phase == OPENING_COMPLETE)
    {
        // TMX map exploration with interactable doors.
        ResolveMapCollisions(player, *room, NULL, 0, NULL, 0);

        // Check if player has moved
        if (!flow->playerHasMovedInOpening && (player->pos.x != player->prevpos.x || player->pos.y != player->prevpos.y)) {
            flow->playerHasMovedInOpening = true;
            flow->movementHintTimer = 0.0f;  // Start fading out
        }

        const NPC* nearbyRondy = FindNearbyRondy(player, npcpool);
        if (flow->phase == OPENING_MAP && nearbyRondy && IsKeyPressed(KEY_E))
        {
            if (!EnsureSceneLoaded(&gRondyOfficeIntroScene, "rondy_office_intro", SCENE_FILE_RONDY_OFFICE)) return true;
            InitScene(scene, &gRondyOfficeIntroScene);
            flow->phase = OPENING_RONDY_DIALOG;
            *shouldEnterScene = true;
            return true;
        }

        if (flow->phase == OPENING_COMPLETE && IsNearInteractable(player, flow->transportZone) && IsKeyPressed(KEY_E))
        {
            *shouldStartTutorial = true;
            return true;
        }

        // Any door tile in TEST MAP.tmx can be interacted with.
        Rectangle nearbyDoor = {0};
        bool isNearDoor = IsDoorHintVisible(player, room->bounds, &nearbyDoor);
        if (isNearDoor) flow->door = nearbyDoor;
        else flow->door = (Rectangle){0};
        if ((flow->phase == OPENING_MAP || flow->phase == OPENING_COMPLETE) && isNearDoor && IsKeyPressed(KEY_E))
        {
            OpenDoorInteractZone(flow->door);
        }

        return true;
    }

    return false;
}

// Draw opening-only prompts: door interactions, Rondy talk prompt, tutorial transport marker.
void DrawOpeningWorldOverlay(const OpeningFlow* flow, const Player* player, const NPCPool* npcpool)
{
    if (!flow || !player || !npcpool) return;

    if (flow->phase == OPENING_MAP || flow->phase == OPENING_COMPLETE)
    {
        if (flow->door.width > 0.0f && flow->door.height > 0.0f && IsNearInteractable(player, flow->door))
        {
            Rectangle nearDoor = flow->door;
            const bool isOpened = IsDoorInteractZoneOpened(nearDoor);
            const char* msg = isOpened ? "Door opened" : "Press E to open door";
            Color msgColor = isOpened ? GREEN : YELLOW;
            float pulse = (sinf((float)GetTime() * 5.5f) + 1.0f) * 0.5f;
            unsigned char alpha = (unsigned char)(155 + pulse * 85.0f);
            Vector2 promptPos = {
                nearDoor.x + nearDoor.width * 0.5f - 48.0f,
                nearDoor.y - 12.0f - pulse * 2.0f
            };
            Color shadow = (Color){20, 20, 20, alpha};
            Color text = (Color){msgColor.r, msgColor.g, msgColor.b, alpha};

            DrawText(msg, (int)promptPos.x + 1, (int)promptPos.y + 1, 14, shadow);
            DrawText(msg, (int)promptPos.x, (int)promptPos.y, 14, text);
        }

        if (flow->phase == OPENING_MAP)
        {
            const NPC* nearbyRondy = FindNearbyRondy(player, npcpool);
            if (nearbyRondy)
            {
                float pulse = (sinf((float)GetTime() * 6.0f) + 1.0f) * 0.5f;
                unsigned char alpha = (unsigned char)(170 + pulse * 75.0f);
                const char* msg = "E: Talk to Rondy";
                Vector2 promptPos = {
                    nearbyRondy->pos.x - 46.0f,
                    nearbyRondy->pos.y - nearbyRondy->body * 2.2f - pulse * 2.0f
                };
                Color shadow = (Color){20, 20, 20, alpha};
                Color text = (Color){120, 255, 140, alpha};

                DrawText(msg, (int)promptPos.x + 1, (int)promptPos.y + 1, 11, shadow);
                DrawText(msg, (int)promptPos.x, (int)promptPos.y, 11, text);
            }
        }

        if (flow->phase == OPENING_COMPLETE)
        {
            bool nearTransport = IsNearInteractable(player, flow->transportZone);
            DrawCircleV(flow->transportPoint, GetMapTileSize() * 0.35f, (Color){95, 235, 170, 120});

            if (nearTransport)
            {
                float pulse = (sinf((float)GetTime() * 6.0f) + 1.0f) * 0.5f;
                unsigned char alpha = (unsigned char)(170 + pulse * 75.0f);
                const char* msg = "E: Enter Tutorial Room";
                Vector2 promptPos = {
                    flow->transportPoint.x - 72.0f,
                    flow->transportPoint.y - GetMapTileSize() * 1.8f - pulse * 2.0f
                };
                Color shadow = (Color){20, 20, 20, alpha};
                Color text = (Color){120, 255, 140, alpha};

                DrawText(msg, (int)promptPos.x + 1, (int)promptPos.y + 1, 11, shadow);
                DrawText(msg, (int)promptPos.x, (int)promptPos.y, 11, text);
            }
        }
    }
}

// Advance opening chapter when active scene ends.
void OpeningHandleSceneComplete(OpeningFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer, NPCPool* npcpool)
{
    if (!flow) return;

    if (flow->phase == OPENING_DIALOG)
    {
        // Narrative briefing complete -> spawn in TMX room.
        EnterOpeningMap(flow, room, player, bulletPool, bulletPoolSize, enemyPool, enemyCapacity, spawnTimer, npcpool);
    }
    else if (flow->phase == OPENING_RONDY_DIALOG)
    {
        flow->phase = OPENING_COMPLETE;
    }
}

// Release scene data cached by opening chapter.
void ShutdownOpeningFlowAssets(void)
{
    UnloadSceneData(&gOpeningWakeNarrationScene);
    UnloadSceneData(&gRondyOfficeIntroScene);
}
