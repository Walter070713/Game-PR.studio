#ifndef COMBAT_RUNTIME_H
#define COMBAT_RUNTIME_H

#include "Enemy.h"
#include "Bullet.h"

// Clear combat entities/timers when entering a peaceful or fresh phase.
void ResetCombatRuntime(Enemy enemyPool[], int enemyCapacity,
    Bullet bulletPool[], int bulletCapacity, float* spawnTimer);

#endif