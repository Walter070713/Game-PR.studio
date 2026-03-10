#ifndef COLLISION_H
#define COLLISION_H
#include "raylib.h"
#include "Bullet.h"
#include "Enemy.h"
void UpdateBulletLife(Bullet* bullet,Rectangle rec,Enemy* enemy);
void UpdateEnemyLife(Bullet* bullet,Enemy* enemy);
#endif