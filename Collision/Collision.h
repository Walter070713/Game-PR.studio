#ifndef COLLISION_H
#define COLLISION_H
#include "raylib.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Player.h"
#include "Map.h"

// Resolve combat collisions: bullets-vs-enemies, player-vs-enemies, enemy-vs-enemy.
void ResolveEnemyCollisions(Player* pl, Enemy e[], int eCount, Bullet b[], int bCount);

// Resolve world collisions against TMX solids and map bounds.
void ResolveMapCollisions(Player* pl, GameMap map,Enemy e[],int eCount,Bullet b[], int bCount);

// Update damage invulnerability and shield regeneration timers.
void UpdatePlayerStats(Player *pl);
#endif