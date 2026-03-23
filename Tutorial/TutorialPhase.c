#include "TutorialPhase.h"
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

void InitTutorialFlow(TutorialFlow* flow)
{
    if (!flow) return;

    flow->isActive = false;
    flow->isComplete = false;
    flow->terminal = (Rectangle){0};
}

void StartTutorialFlow(TutorialFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer)
{
    if (!flow || !room || !player || !bulletPool || !enemyPool || !spawnTimer) return;

    *room = InitTutorialRoom();
    flow->terminal = (Rectangle){
        room->bounds.x + room->bounds.width - 320.0f,
        room->bounds.y + room->bounds.height * 0.5f - 90.0f,
        180.0f,
        180.0f
    };

    player->pos = (Vector2){room->bounds.x + 180.0f, room->bounds.y + room->bounds.height * 0.5f};
    player->prevpos = player->pos;

    InitBulletPool(bulletPool, bulletPoolSize);
    ResetEnemyPoolToInactive(enemyPool, enemyCapacity);
    *spawnTimer = 0.0f;

    flow->isActive = true;
    flow->isComplete = false;
}

bool UpdateTutorialFlow(TutorialFlow* flow, Player* player, GameMap* room,
    Enemy enemyPool[], Bullet bulletPool[])
{
    if (!flow || !player || !room || !enemyPool || !bulletPool || !flow->isActive) return false;

    ResolveMapCollisions(player, *room, enemyPool, 0, bulletPool, 0);

    bool nearTerminal = CheckCollisionCircleRec(player->pos, player->body + 20.0f, flow->terminal);
    if (nearTerminal && IsKeyPressed(KEY_E))
    {
        flow->isComplete = true;
        flow->isActive = false;
    }

    return true;
}

void DrawTutorialWorldOverlay(const TutorialFlow* flow)
{
    if (!flow || !flow->isActive) return;

    DrawRectangleRec(flow->terminal, (Color){120, 90, 150, 255});
    DrawRectangleLinesEx(flow->terminal, 3.0f, VIOLET);
}

void DrawTutorialHUD(const TutorialFlow* flow, const Player* player)
{
    if (!flow || !player) return;

    if (flow->isActive)
    {
        bool nearTerminal = CheckCollisionCircleRec(player->pos, player->body + 20.0f, flow->terminal);
        DrawText("Tutorial Objective: Reach the terminal", 10, 230, 30, ORANGE);

        if (nearTerminal)
        {
            DrawText("Press E to finish tutorial", 10, 270, 30, GREEN);
        }
        else
        {
            DrawText("Move to the purple terminal", 10, 270, 30, LIGHTGRAY);
        }
    }
    else if (flow->isComplete)
    {
        DrawText("Tutorial Complete", 10, 230, 30, GREEN);
    }
}
