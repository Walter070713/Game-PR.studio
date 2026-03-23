#include "OpeningPhase.h"
#include "SceneData.h"
#include "Collision.h"

// Shared tuning values for opening interaction range and reminder display.
static const float kInteractPadding = 20.0f;
static const float kReminderSeconds = 2.2f;

// Helper for circular interaction checks against rectangular interactables.
static bool IsNearInteractable(const Player* player, Rectangle target)
{
    if (!player) return false;
    return CheckCollisionCircleRec(player->pos, player->body + kInteractPadding, target);
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

void InitOpeningFlow(OpeningFlow* flow)
{
    if (!flow) return;

    // Default to inactive until Start Mission is triggered.
    flow->phase = OPENING_NOT_STARTED;
    flow->door = (Rectangle){0};
    flow->interactBlock = (Rectangle){0};
    flow->hasInteractedWithBlock = false;
    flow->reminderTimer = 0.0f;
}

void OpeningStartMission(OpeningFlow* flow, Scene* scene)
{
    if (!flow || !scene) return;

    // Start opening narrative scene and enter opening chapter state.
    InitScene(scene, &OpeningScene);
    flow->phase = OPENING_DIALOG;
}

// Internal setup for the first playable opening room.
static void EnterOpeningSmallRoom(OpeningFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer)
{
    if (!flow || !room || !player || !bulletPool || !enemyPool || !spawnTimer) return;

    *room = InitOpeningSmallRoom();

    // Door unlock is gated by the console interaction.
    flow->door = (Rectangle){
        room->bounds.x + room->bounds.width - 20.0f,
        room->bounds.y + room->bounds.height * 0.5f - 130.0f,
        24.0f,
        260.0f
    };
    flow->interactBlock = (Rectangle){
        room->bounds.x + 420.0f,
        room->bounds.y + room->bounds.height * 0.5f - 70.0f,
        140.0f,
        140.0f
    };
    flow->hasInteractedWithBlock = false;
    flow->reminderTimer = 0.0f;

    player->pos = (Vector2){room->bounds.x + 180.0f, room->bounds.y + room->bounds.height * 0.5f};
    player->prevpos = player->pos;

    InitBulletPool(bulletPool, bulletPoolSize);
    ResetEnemyPoolToInactive(enemyPool, enemyCapacity);
    *spawnTimer = 0.0f;

    flow->phase = OPENING_SMALL_ROOM;
}

bool UpdateOpeningPeacefulPhase(OpeningFlow* flow, Player* player, GameMap* room,
    Enemy enemyPool[], int enemyCapacity, Bullet bulletPool[], float* spawnTimer, Scene* scene,
    bool* shouldEnterScene)
{
    if (!flow || !player || !room || !enemyPool || !bulletPool || !spawnTimer || !scene || !shouldEnterScene) return false;

    // Output flag tells main.c whether to switch from gameplay to scene mode.
    *shouldEnterScene = false;

    // Decay door reminder text timer every frame.
    if (flow->reminderTimer > 0.0f)
    {
        flow->reminderTimer -= GetFrameTime();
        if (flow->reminderTimer < 0.0f) flow->reminderTimer = 0.0f;
    }

    if (flow->phase == OPENING_SMALL_ROOM)
    {
        // Keep opening room peaceful: map collision only.
        ResolveMapCollisions(player, *room, enemyPool, 0, bulletPool, 0);

        // Block interaction triggers a short dialog scene.
        bool isNearBlock = IsNearInteractable(player, flow->interactBlock);
        if (!flow->hasInteractedWithBlock && isNearBlock && IsKeyPressed(KEY_E))
        {
            InitScene(scene, &OpeningBlockInteractScene);
            flow->phase = OPENING_BLOCK_DIALOG;
            *shouldEnterScene = true;
            return true;
        }

        // Door interaction remains locked until block interaction is done.
        bool isNearDoor = IsNearInteractable(player, flow->door);
        if (isNearDoor && IsKeyPressed(KEY_E))
        {
            if (!flow->hasInteractedWithBlock)
            {
                flow->reminderTimer = kReminderSeconds;
                return true;
            }

            *room = InitOpeningBigRoom();
            flow->phase = OPENING_BIG_ROOM;
            flow->reminderTimer = 0.0f;

            player->pos = (Vector2){room->bounds.x + 220.0f, room->bounds.y + room->bounds.height * 0.5f};
            player->prevpos = player->pos;

            ResetEnemyPoolToInactive(enemyPool, enemyCapacity);
            *spawnTimer = 0.0f;
        }

        return true;
    }

    if (flow->phase == OPENING_BLOCK_DIALOG)
    {
        // Scene system owns this temporary phase.
        return true;
    }

    if (flow->phase == OPENING_BIG_ROOM)
    {
        // Big room remains peaceful until tutorial handoff.
        ResolveMapCollisions(player, *room, enemyPool, 0, bulletPool, 0);
        return true;
    }

    return false;
}

bool OpeningIsPeacefulPhase(const OpeningFlow* flow)
{
    if (!flow) return false;
    return flow->phase == OPENING_SMALL_ROOM || flow->phase == OPENING_BIG_ROOM;
}

bool OpeningIsCombatEnabled(const OpeningFlow* flow)
{
    if (!flow) return true;
    return !OpeningIsPeacefulPhase(flow) && flow->phase != OPENING_BLOCK_DIALOG;
}

bool OpeningIsComplete(const OpeningFlow* flow)
{
    if (!flow) return false;
    return flow->phase == OPENING_BIG_ROOM;
}

void DrawOpeningWorldOverlay(const OpeningFlow* flow)
{
    if (!flow) return;

    if (flow->phase == OPENING_SMALL_ROOM)
    {
        // Render interaction anchors for opening objective.
        DrawRectangleRec(flow->interactBlock, (Color){70, 110, 150, 255});
        DrawRectangleLinesEx(flow->interactBlock, 3.0f, flow->hasInteractedWithBlock ? GREEN : SKYBLUE);
        DrawRectangleRec(flow->door, (Color){180, 120, 60, 255});
        DrawRectangleLinesEx(flow->door, 3.0f, YELLOW);
    }
}

void DrawOpeningHUD(const OpeningFlow* flow, const Player* player)
{
    if (!flow || !player) return;

    if (flow->phase == OPENING_SMALL_ROOM)
    {
        // Contextual instructions based on current opening objective.
        bool isNearBlock = IsNearInteractable(player, flow->interactBlock);
        bool isNearDoor = IsNearInteractable(player, flow->door);

        if (!flow->hasInteractedWithBlock)
        {
            DrawText("Opening Objective: Interact with the console block", 10, 230, 30, ORANGE);
            if (isNearBlock)
            {
                DrawText("Press E to inspect the block", 10, 270, 30, GREEN);
            }
            else
            {
                DrawText("Move to the blue block first", 10, 270, 30, LIGHTGRAY);
            }
        }
        else
        {
            DrawText("Opening Objective: Find the door", 10, 230, 30, ORANGE);
            if (isNearDoor)
            {
                DrawText("Press E to interact with door", 10, 270, 30, GREEN);
            }
            else
            {
                DrawText("Move to the highlighted door", 10, 270, 30, LIGHTGRAY);
            }
        }

        if (flow->reminderTimer > 0.0f)
        {
            DrawText("Door locked. Inspect the console block first.", 10, 310, 30, RED);
        }
    }
    else if (flow->phase == OPENING_BIG_ROOM)
    {
        DrawText("Opening Complete: You entered the main room", 10, 230, 30, GREEN);
    }
}

void OpeningHandleSceneComplete(OpeningFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer)
{
    if (!flow) return;

    if (flow->phase == OPENING_DIALOG)
    {
        // Narrative briefing complete -> spawn in opening small room.
        EnterOpeningSmallRoom(flow, room, player, bulletPool, bulletPoolSize, enemyPool, enemyCapacity, spawnTimer);
    }
    else if (flow->phase == OPENING_BLOCK_DIALOG)
    {
        // Console dialog complete -> unlock door objective.
        flow->hasInteractedWithBlock = true;
        flow->phase = OPENING_SMALL_ROOM;
        flow->reminderTimer = 0.0f;
    }
}
