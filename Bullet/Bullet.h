#ifndef BULLET_H
#define BULLET_H
#include "raylib.h"
#include "raymath.h"

typedef struct Bullet{
    Vector2 pos;
    Vector2 dir;
    float size;
    float speed;
    Color color;
    bool active;
}Bullet;

// Initialize bullet pool
void InitBulletPool(Bullet bulletpool[], int capacity);

// Update bullet physics (movement and lifetime)
void UpdateBulletPhysics(Bullet bulletpool[], int capacity, Vector2 playerPos);

// Fire a bullet from player position with given direction
void FireBullet(Bullet bulletpool[], int capacity, Vector2 startPos, Vector2 direction, float speed, float size, Color color);

// Draw all active bullets
void DrawBullet(Bullet bulletpool[], int capacity);

#endif