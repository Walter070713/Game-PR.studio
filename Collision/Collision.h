#ifndef COLLISION_H
#define COLLISION_H
#include "raylib.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Player.h"
void CheckBulletEnemyCollisions(Bullet b[], int bCount, Enemy e[], int eCount);
// void CheckBulletBlockCollisions(Bullet b[], int bCount);
// void CheckPlayerEnemyCollisions(Bullet pool[], int bCount, Enemy enemies[], int eCount);
// void CheckPlayerBlockCollisions(Bullet pool[], int bCount, Enemy enemies[], int eCount);
// void CheckEnemyBlockCollisions(Bullet pool[], int bCount, Enemy enemies[], int eCount);
#endif