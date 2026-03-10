#ifndef COLLISION_H
#define COLLISION_H
#include "raylib.h"
#include "Enemy.h"
#include "Bullet.h"
void UpdateBulletLife(Bullet* bullet,Rectangle rec,Enemy* enemy);
void UpdateEnemyLife(Enemy* enemy);
#endif