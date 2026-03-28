#include "CombatRuntime.h"

// Reset combat pools and timers when entering non-combat or fresh phase.
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
