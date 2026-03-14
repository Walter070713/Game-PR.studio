#ifndef COLLISION_H
#define COLLISION_H
#include "raylib.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Player.h"
#include "Map.h"
void ResolveEnemyCollisions(Player* pl, Enemy e[], int eCount, Bullet b[], int bCount);
void ResolveMapCollisions(Player* pl, GameMap map,Enemy e[],int eCount);
#endif