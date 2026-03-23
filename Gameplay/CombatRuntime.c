#include "CombatRuntime.h"
#include "Weapon.h"
#include "Spawn.h"
#include "Collision.h"

void UpdateCombatRuntime(Player* player, Enemy enemyPool[], int enemyCapacity,
    Bullet bulletPool[], int bulletCapacity, MseAim* mouse, Camera2D camera,
    GameMap room, float* spawnTimer, float spawnRate)
{
    if (!player || !enemyPool || !bulletPool || !mouse || !spawnTimer) return;

    UpdateMouseAim(mouse, camera, player->pos);
    UpdateWeapon(&player->weapon);
    UpdatePlayerStats(player);

    UpdateSpawner(enemyPool, enemyCapacity, player->pos, spawnTimer, spawnRate, room);
    UpdateEnemyHorde(enemyPool, enemyCapacity, player->pos);
    UpdateBulletPhysics(bulletPool, bulletCapacity, player->pos);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && FireWeapon(&player->weapon))
    {
        FireBullet(bulletPool, bulletCapacity, player->pos, mouse->dir,
            player->weapon.bulletSpeed, player->weapon.bulletSize, player->weapon.bulletColor);
    }

    ResolveEnemyCollisions(player, enemyPool, enemyCapacity, bulletPool, bulletCapacity);
    ResolveMapCollisions(player, room, enemyPool, enemyCapacity, bulletPool, bulletCapacity);
}

void DrawCombatRuntimeWorld(Player* player, Enemy enemyPool[], int enemyCapacity,
    Bullet bulletPool[], int bulletCapacity, MseAim* mouse)
{
    if (!player || !enemyPool || !bulletPool || !mouse) return;

    DrawEnemy(enemyPool, enemyCapacity);

    {
        float screenScaleX = (float)GetScreenWidth() / 2560.0f;
        float screenScaleY = (float)GetScreenHeight() / 1600.0f;
        float screenScale = (screenScaleX < screenScaleY) ? screenScaleX : screenScaleY;
        Vector2 weaponEnd = Vector2Add(player->pos, Vector2Scale(mouse->dir, 50.0f * screenScale));

        DrawLineEx(player->pos, weaponEnd, 8.0f * screenScale, RED);
    }

    DrawBullet(bulletPool, bulletCapacity);
}

void DrawCombatRuntimeHUD(Player* player)
{
    if (!player) return;

    DrawText(TextFormat("Health: %d", player->health), 10, 60, 30, RED);
    DrawText(TextFormat("Shield: %d", player->shield), 10, 100, 30, BLUE);

    WeaponInfo winfo = GetWeaponInfo(&player->weapon);
    DrawText(TextFormat("Magazine: %d/%d", winfo.magazine, player->weapon.maxMagazine), 10, 150, 30, YELLOW);
    DrawText(TextFormat("Total Ammo: %d", winfo.totalAmmo), 10, 190, 30, YELLOW);
    DrawReload(&winfo);
}

void ResetCombatRuntime(Enemy enemyPool[], int enemyCapacity,
    Bullet bulletPool[], int bulletCapacity, float* spawnTimer)
{
    if (!enemyPool || !bulletPool || !spawnTimer) return;

    for (int i = 0; i < enemyCapacity; i++)
    {
        enemyPool[i].active = false;
        enemyPool[i].health = 0;
        enemyPool[i].flashtime = 0.0f;
    }

    InitBulletPool(bulletPool, bulletCapacity);
    *spawnTimer = 0.0f;
}
