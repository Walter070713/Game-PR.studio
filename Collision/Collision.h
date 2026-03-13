#ifndef COLLISION_H
#define COLLISION_H
#include "raylib.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Player.h"
void ResolveAllCollisions(Player* pl, Enemy e[], int eCount, Bullet b[], int bCount);
#endif