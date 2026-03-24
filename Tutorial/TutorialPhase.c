#include "TutorialPhase.h"
#include "Collision.h"
#include "MouseAim.h"
#include "Weapon.h"

static const float kTutorialSpawnDelaySeconds = 2.0f;
static const int kTutorialEnemySpawnCount = 4;
static const float kTutorialEnemySpeed = 68.0f;
static const int kTutorialEnemyHealth = 6;
static const float kTutorialEnemySpawnMaxX = 1203.0f;
static const Vector2 kTutorialPlayerSpawnPoint = {1203.0f, 1157.0f};
static const int kTutorialTotalWaves = 2;

static bool HasActiveEnemies(const Enemy enemyPool[], int enemyCapacity)
{
    if (!enemyPool || enemyCapacity <= 0) return false;
    for (int i = 0; i < enemyCapacity; i++)
    {
        if (enemyPool[i].active && enemyPool[i].health > 0) return true;
    }
    return false;
}

static Vector2 FindNearestWalkablePoint(GameMap room, Vector2 preferred)
{
    float tile = GetMapTileSize();
    if (tile < 1.0f) tile = 16.0f;

    if (!IsMapPointBlocked(preferred, room.bounds)) return preferred;

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
                    preferred.x + (float)ox * tile,
                    preferred.y + (float)oy * tile
                };

                if (candidate.x <= room.bounds.x || candidate.y <= room.bounds.y ||
                    candidate.x >= room.bounds.x + room.bounds.width ||
                    candidate.y >= room.bounds.y + room.bounds.height)
                {
                    continue;
                }

                if (!IsMapPointBlocked(candidate, room.bounds)) return candidate;
            }
        }
    }

    return preferred;
}

static Vector2 FindEnemySpawnPoint(GameMap room, Vector2 preferred)
{
    float tile = GetMapTileSize();
    if (tile < 1.0f) tile = 16.0f;

    if (preferred.x > kTutorialEnemySpawnMaxX - tile)
    {
        preferred.x = kTutorialEnemySpawnMaxX - tile;
    }

    return FindNearestWalkablePoint(room, preferred);
}

static void SpawnTutorialEnemies(TutorialFlow* flow, GameMap room, Enemy enemyPool[], int enemyCapacity, const Player* player)
{
    if (!flow || !enemyPool || enemyCapacity <= 0 || !player) return;

    float tile = GetMapTileSize();
    float spacing = tile * 2.2f;
    Vector2 center = {
        room.bounds.x + tile * 11.0f,
        room.bounds.y + room.bounds.height * 0.52f
    };

    int spawned = 0;
    for (int i = 0; i < enemyCapacity && spawned < kTutorialEnemySpawnCount; i++)
    {
        Vector2 preferred = {
            center.x + (float)(spawned - 1) * spacing,
            center.y + (float)((spawned % 2) == 0 ? -1 : 1) * tile * 1.3f
        };
        Vector2 spawnPos = FindEnemySpawnPoint(room, preferred);

        enemyPool[i].active = true;
        enemyPool[i].health = kTutorialEnemyHealth;
        enemyPool[i].flashtime = 0.0f;
        enemyPool[i].pos = spawnPos;
        enemyPool[i].prevpos = spawnPos;
        enemyPool[i].body = player->body * 1.25f;
        enemyPool[i].speed = kTutorialEnemySpeed;
        enemyPool[i].state = WHITE;
        enemyPool[i].name = 0;
        spawned++;
    }

    flow->enemiesSpawned = true;
    flow->hintTimer = 6.0f;
}

// Tutorial remains peaceful, so enemy pool is disabled while active.
static void ResetEnemyPoolToInactive(Enemy enemyPool[], int enemyCapacity)
{
    // Initialize via shared enemy system first, then disable until wave spawn.
    InitEnemy(enemyPool, enemyCapacity);

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

    // Inactive by default until opening chapter promotes to tutorial.
    flow->isActive = false;
    flow->enemiesSpawned = false;
    flow->spawnDelayTimer = kTutorialSpawnDelaySeconds;
    flow->hintTimer = 0.0f;
    flow->currentWave = 0;
    flow->totalWaves = kTutorialTotalWaves;
}

void StartTutorialFlow(TutorialFlow* flow, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer)
{
    if (!flow || !room || !player || !bulletPool || !enemyPool || !spawnTimer) return;

    // Switch to dedicated tutorial TMX map before creating room bounds.
    SetActiveTMXMap("TUTORIAL MAP.tmx");
    *room = InitRoom();

    player->pos = kTutorialPlayerSpawnPoint;
    player->prevpos = player->pos;

    // Keep the player/weapon scale consistent with TMX tile size in tutorial.
    player->body = GetMapTileSize() * 0.31f;

    player->weapon = InitWeapon(WEAPON_PISTOL);
    player->weapon.bulletSize = player->body * 0.34f;
    player->weapon.bulletSpeed = player->speed * 5.2f;
    if (player->weapon.bulletSpeed < GetMapTileSize() * 30.0f)
    {
        player->weapon.bulletSpeed = GetMapTileSize() * 30.0f;
    }
    player->weapon.lastFireTime = player->weapon.fireRate;

    // Reset combat runtime pools because tutorial does not use combat.
    InitBulletPool(bulletPool, bulletPoolSize);
    ResetEnemyPoolToInactive(enemyPool, enemyCapacity);
    *spawnTimer = 0.0f;

    flow->isActive = true;
    flow->enemiesSpawned = false;
    flow->spawnDelayTimer = kTutorialSpawnDelaySeconds;
    flow->hintTimer = 0.0f;
    flow->currentWave = 0;
    flow->totalWaves = kTutorialTotalWaves;
}

bool UpdateTutorialFlow(TutorialFlow* flow, Player* player, GameMap* room,
    Enemy enemyPool[], int enemyCapacity, Bullet bulletPool[], int bulletCapacity, Camera2D camera)
{
    if (!flow || !player || !room || !enemyPool || !bulletPool || !flow->isActive) return false;

    UpdateWeapon(&player->weapon);
    UpdatePlayerStats(player);

    {
        MseAim mouse = {0};
        Vector2 shootDir = {1.0f, 0.0f};
        UpdateMouseAim(&mouse, camera, player->pos);

        if (Vector2LengthSqr(mouse.dir) > 0.0001f)
        {
            shootDir = mouse.dir;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && FireWeapon(&player->weapon))
        {
            FireBullet(bulletPool, bulletCapacity, player->pos, shootDir,
                player->weapon.bulletSpeed, player->weapon.bulletSize, player->weapon.bulletColor);
        }
    }

    if (!flow->enemiesSpawned && flow->currentWave < flow->totalWaves)
    {
        flow->spawnDelayTimer -= GetFrameTime();
        if (flow->spawnDelayTimer <= 0.0f)
        {
            SpawnTutorialEnemies(flow, *room, enemyPool, enemyCapacity, player);
            flow->currentWave++;
        }
    }

    UpdateEnemyHorde(enemyPool, enemyCapacity, player->pos);
    UpdateBulletPhysics(bulletPool, bulletCapacity, player->pos);
    ResolveEnemyCollisions(player, enemyPool, enemyCapacity, bulletPool, bulletCapacity);
    ResolveMapCollisions(player, *room, enemyPool, enemyCapacity, bulletPool, bulletCapacity);

    if (flow->enemiesSpawned && !HasActiveEnemies(enemyPool, enemyCapacity))
    {
        flow->enemiesSpawned = false;
        if (flow->currentWave < flow->totalWaves)
        {
            flow->spawnDelayTimer = kTutorialSpawnDelaySeconds;
        }
    }

    if (flow->hintTimer > 0.0f)
    {
        flow->hintTimer -= GetFrameTime();
        if (flow->hintTimer < 0.0f) flow->hintTimer = 0.0f;
    }

    return true;
}

void DrawTutorialWorldOverlay(const TutorialFlow* flow, const Player* player, Enemy enemyPool[], int enemyCapacity,
    Bullet bulletPool[], int bulletCapacity, Camera2D camera)
{
    (void)flow;
    DrawEnemy(enemyPool, enemyCapacity);
    DrawBullet(bulletPool, bulletCapacity);

    if (player)
    {
        MseAim mouse = {0};
        Vector2 aimDir = {1.0f, 0.0f};
        UpdateMouseAim(&mouse, camera, player->pos);

        if (Vector2LengthSqr(mouse.dir) > 0.0001f)
        {
            aimDir = mouse.dir;
        }
        Vector2 weaponEnd = Vector2Add(player->pos, Vector2Scale(aimDir, player->body * 2.2f));
        DrawLineEx(player->pos, weaponEnd, player->body * 0.4f, RED);
    }
}

void DrawTutorialHUD(const TutorialFlow* flow, const Player* player)
{
    (void)player;
    if (!flow || !flow->isActive) return;

    if (!flow->enemiesSpawned)
    {
        if (flow->currentWave < flow->totalWaves)
        {
            DrawText(TextFormat("Get ready... Wave %d incoming", flow->currentWave + 1), 24, 230, 34, ORANGE);
        }
        else
        {
            DrawText("All tutorial waves cleared", 24, 230, 34, GREEN);
        }
        return;
    }

    {
        float bob = sinf((float)GetTime() * 4.0f) * 8.0f;
        int cx = GetScreenWidth() / 2;
        const char* h1 = "MOUSE: AIM";
        const char* h2 = "LEFT CLICK: SHOOT";
        const char* h3 = "R: RELOAD";
        int f = 42;
        int y = (int)(120.0f + bob);

        int w1 = MeasureText(h1, f);
        int w2 = MeasureText(h2, f);
        int w3 = MeasureText(h3, f);
        DrawText(h1, cx - w1 / 2 + 2, y + 2, f, (Color){20, 20, 20, 220});
        DrawText(h1, cx - w1 / 2, y, f, (Color){255, 235, 120, 255});
        DrawText(h2, cx - w2 / 2 + 2, y + 52, f, (Color){20, 20, 20, 220});
        DrawText(h2, cx - w2 / 2, y + 50, f, (Color){255, 235, 120, 255});
        DrawText(h3, cx - w3 / 2 + 2, y + 102, f, (Color){20, 20, 20, 220});
        DrawText(h3, cx - w3 / 2, y + 100, f, (Color){255, 235, 120, 255});
        DrawText(TextFormat("Wave %d/%d", flow->currentWave, flow->totalWaves), 24, 24, 30, (Color){235, 240, 245, 255});

        if (flow->hintTimer <= 0.0f)
        {
            DrawText("Eliminate them", cx - 140, y + 154, 34, (Color){250, 160, 110, 255});
        }
    }
}
