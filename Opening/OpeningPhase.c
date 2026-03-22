#include "OpeningPhase.h"
#include "SceneData.h"
#include "Collision.h"

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

    flow->phase = OPENING_NOT_STARTED;
    flow->door = (Rectangle){0};
}

void OpeningStartMission(OpeningFlow* flow, Scene* scene)
{
    if (!flow || !scene) return;

    InitScene(scene, &OpeningScene);
    flow->phase = OPENING_DIALOG;
}

void OpeningEnterSmallRoomAfterDialog(OpeningFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer)
{
    if (!flow || !room || !player || !bulletPool || !enemyPool || !spawnTimer) return;

    *room = InitOpeningSmallRoom();
    flow->door = (Rectangle){
        room->bounds.x + room->bounds.width - 20.0f,
        room->bounds.y + room->bounds.height * 0.5f - 130.0f,
        24.0f,
        260.0f
    };

    player->pos = (Vector2){room->bounds.x + 180.0f, room->bounds.y + room->bounds.height * 0.5f};
    player->prevpos = player->pos;

    InitBulletPool(bulletPool, bulletPoolSize);
    ResetEnemyPoolToInactive(enemyPool, enemyCapacity);
    *spawnTimer = 0.0f;

    flow->phase = OPENING_SMALL_ROOM;
}

bool UpdateOpeningPeacefulPhase(OpeningFlow* flow, Player* player, GameMap* room,
    Enemy enemyPool[], int enemyCapacity, Bullet bulletPool[], float* spawnTimer)
{
    if (!flow || !player || !room || !enemyPool || !bulletPool || !spawnTimer) return false;

    if (flow->phase == OPENING_SMALL_ROOM)
    {
        ResolveMapCollisions(player, *room, enemyPool, 0, bulletPool, 0);

        bool isNearDoor = CheckCollisionCircleRec(player->pos, player->body + 20.0f, flow->door);
        if (isNearDoor && IsKeyPressed(KEY_E))
        {
            *room = InitOpeningBigRoom();
            flow->phase = OPENING_BIG_ROOM;

            player->pos = (Vector2){room->bounds.x + 220.0f, room->bounds.y + room->bounds.height * 0.5f};
            player->prevpos = player->pos;

            ResetEnemyPoolToInactive(enemyPool, enemyCapacity);
            *spawnTimer = 0.0f;
        }

        return true;
    }

    if (flow->phase == OPENING_BIG_ROOM)
    {
        ResolveMapCollisions(player, *room, enemyPool, 0, bulletPool, 0);
        return true;
    }

    return false;
}

bool OpeningIsCombatEnabled(const OpeningFlow* flow)
{
    if (!flow) return true;
    return flow->phase != OPENING_SMALL_ROOM && flow->phase != OPENING_BIG_ROOM;
}

void DrawOpeningWorldOverlay(const OpeningFlow* flow)
{
    if (!flow) return;

    if (flow->phase == OPENING_SMALL_ROOM)
    {
        DrawRectangleRec(flow->door, (Color){180, 120, 60, 255});
        DrawRectangleLinesEx(flow->door, 3.0f, YELLOW);
    }
}

void DrawOpeningHUD(const OpeningFlow* flow, const Player* player)
{
    if (!flow || !player) return;

    if (flow->phase == OPENING_SMALL_ROOM)
    {
        bool isNearDoor = CheckCollisionCircleRec(player->pos, player->body + 20.0f, flow->door);
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
    else if (flow->phase == OPENING_BIG_ROOM)
    {
        DrawText("Opening Complete: You entered the main room", 10, 230, 30, GREEN);
    }
}
