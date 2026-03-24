#include "OpeningPhase.h"
#include "SceneData.h"
#include "Collision.h"

// Shared tuning values for opening interaction range and reminder display.
static const float kDoorInteractPadding = 24.0f;
static const float kReminderSeconds = 2.2f;
static const Vector2 kTutorialTransportPoint = {1215.0f, 1024.0f};
static const Vector2 kOpeningDoorPoints[] = {
    {1159.0f, 1216.0f},
    {1295.0f, 1154.0f},
    {1215.0f, 1083.0f}
};
static const int kOpeningDoorCount = (int)(sizeof(kOpeningDoorPoints) / sizeof(kOpeningDoorPoints[0]));

static float DistancePointToRectLocal(Vector2 p, Rectangle r)
{
    float dx = 0.0f;
    float dy = 0.0f;

    if (p.x < r.x) dx = r.x - p.x;
    else if (p.x > r.x + r.width) dx = p.x - (r.x + r.width);

    if (p.y < r.y) dy = r.y - p.y;
    else if (p.y > r.y + r.height) dy = p.y - (r.y + r.height);

    return sqrtf(dx * dx + dy * dy);
}

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

static bool IsDoorHintVisible(const Player* player, Rectangle mapBounds, Rectangle* outDoor)
{
    (void)mapBounds;
    if (!player || !outDoor) return false;

    {
        float tile = GetMapTileSize();
        float bestDist = tile * 7.0f;
        bool found = false;
        Rectangle nearest = {0};

        for (int i = 0; i < kOpeningDoorCount; i++)
        {
            float zoneW = tile * 2.0f;
            float zoneH = tile * 2.0f;
            float zoneYOffset = 0.0f;

            // Top-middle door is blocked by nearby tiles; give it a wider/down-shifted interaction box.
            if (i == 2)
            {
                zoneW = tile * 3.2f;
                zoneH = tile * 3.2f;
                zoneYOffset = tile * 0.9f;
            }

            Rectangle zone = {
                kOpeningDoorPoints[i].x - zoneW * 0.5f,
                kOpeningDoorPoints[i].y - zoneH * 0.5f + zoneYOffset,
                zoneW,
                zoneH
            };

            float dist = DistancePointToRectLocal(player->pos, zone);
            if (dist <= bestDist)
            {
                bestDist = dist;
                nearest = zone;
                found = true;
            }
        }

        if (!found) return false;
        *outDoor = nearest;
        return true;
    }
}

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

// Opening phases never spawn enemies, so keep pool inactive.
static void ResetEnemyPoolToInactive(Enemy enemyPool[], int enemyCapacity)
{
    for (int i = 0; i < enemyCapacity; i++)
    {
        enemyPool[i].active = false;
        enemyPool[i].health = 0;
        enemyPool[i].flashtime = 0.0f;
    }
}

// Forward declaration
static void EnterOpeningMap(OpeningFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer, NPCPool* npcpool);

void InitOpeningFlow(OpeningFlow* flow)
{
    if (!flow) return;

    // Default to inactive until Start Mission is triggered.
    flow->phase = OPENING_NOT_STARTED;
    flow->door = (Rectangle){0};
    flow->transportZone = (Rectangle){0};
    flow->transportPoint = kTutorialTransportPoint;
    flow->reminderTimer = 0.0f;
    flow->movementHintTimer = 5.0f;  // Show hint for 5 seconds initially
    flow->playerHasMovedInOpening = false;
}

void OpeningStartMission(OpeningFlow* flow, Scene* scene, GameMap* room, Player* player, 
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer, NPCPool* npcpool)
{
    if (!flow || !scene || !room || !player || !bulletPool || !enemyPool || !spawnTimer || !npcpool) return;

    (void)room;
    (void)player;
    (void)bulletPool;
    (void)bulletPoolSize;
    (void)enemyPool;
    (void)enemyCapacity;
    (void)spawnTimer;
    (void)npcpool;

    if (player && player->name && player->name[0] != '\0')
    {
        OpeningWakeNarrationLines[1].characterName = player->name;
    }
    else
    {
        OpeningWakeNarrationLines[1].characterName = "Player";
    }

    SetActiveTMXMap("TEST MAP.tmx");

    InitScene(scene, &OpeningWakeNarrationScene);
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

    flow->reminderTimer = 0.0f;

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

    InitBulletPool(bulletPool, bulletPoolSize);
    ResetEnemyPoolToInactive(enemyPool, enemyCapacity);
    *spawnTimer = 0.0f;

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

bool UpdateOpeningPeacefulPhase(OpeningFlow* flow, Player* player, GameMap* room,
    Enemy enemyPool[], int enemyCapacity, Bullet bulletPool[], float* spawnTimer, Scene* scene, NPCPool* npcpool,
    bool* shouldEnterScene, bool* shouldStartTutorial)
{
    if (!flow || !player || !room || !enemyPool || !bulletPool || !spawnTimer || !scene || !npcpool || !shouldEnterScene || !shouldStartTutorial) return false;

    // Output flag tells main.c whether to switch from gameplay to scene mode.
    *shouldEnterScene = false;
    *shouldStartTutorial = false;

    // Decay door reminder text timer every frame.
    if (flow->reminderTimer > 0.0f)
    {
        flow->reminderTimer -= GetFrameTime();
        if (flow->reminderTimer < 0.0f) flow->reminderTimer = 0.0f;
    }

    if (flow->phase == OPENING_MAP || flow->phase == OPENING_COMPLETE)
    {
        // TMX map exploration with interactable doors.
        ResolveMapCollisions(player, *room, enemyPool, 0, bulletPool, 0);

        // Check if player has moved
        if (!flow->playerHasMovedInOpening && (player->pos.x != player->prevpos.x || player->pos.y != player->prevpos.y)) {
            flow->playerHasMovedInOpening = true;
            flow->movementHintTimer = 0.0f;  // Start fading out
        }

        const NPC* nearbyRondy = FindNearbyRondy(player, npcpool);
        if (flow->phase == OPENING_MAP && nearbyRondy && IsKeyPressed(KEY_E))
        {
            InitScene(scene, &RondyOfficeIntroScene);
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
            flow->reminderTimer = 0.0f;
        }

        return true;
    }

    return false;
}

bool OpeningIsPeacefulPhase(const OpeningFlow* flow)
{
    if (!flow) return false;
    return flow->phase == OPENING_MAP || flow->phase == OPENING_COMPLETE;
}

bool OpeningIsCombatEnabled(const OpeningFlow* flow)
{
    if (!flow) return true;
    return false;
}

bool OpeningIsComplete(const OpeningFlow* flow)
{
    if (!flow) return false;
    return flow->phase == OPENING_COMPLETE;
}

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

void DrawOpeningHUD(const OpeningFlow* flow, const Player* player)
{
    (void)flow;
    (void)player;
}

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
