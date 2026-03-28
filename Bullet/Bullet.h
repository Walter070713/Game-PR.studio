#ifndef BULLET_H
#define BULLET_H
#include "raylib.h"

// One pooled projectile instance.
typedef struct Bullet{
    Vector2 pos;
    Vector2 dir;
    float size;
    float speed;
    Color color;
    bool active;
}Bullet;

// Reset every slot to an inactive/default state.
void InitBulletPool(Bullet bulletpool[], int capacity);

// Advance active bullets and deactivate ones outside lifetime radius.
void UpdateBulletPhysics(Bullet bulletpool[], int capacity, Vector2 playerPos);

// Spawn one bullet into the first free slot.
void FireBullet(Bullet bulletpool[], int capacity, Vector2 startPos, Vector2 direction, float speed, float size, Color color);

// Render all active bullets.
void DrawBullet(Bullet bulletpool[], int capacity);

#endif