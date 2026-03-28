#include "Bullet.h"
#include "raymath.h"

// Bullets are retired when they drift too far from player origin.
#define BULLET_MAX_DISTANCE_SQ 2000000.0f

// Normalize a slot back to default values.
static void ResetBullet(Bullet* bullet)
{
    if (!bullet) return;

    bullet->active = false;
    bullet->speed = 0.0f;
    bullet->size = 0.0f;
    bullet->pos = (Vector2){0.0f, 0.0f};
    bullet->dir = (Vector2){0.0f, 0.0f};
    bullet->color = WHITE;
}

// Find first reusable slot in the bullet pool.
static int FindInactiveBulletIndex(Bullet bulletpool[], int capacity)
{
    for (int i = 0; i < capacity; ++i)
    {
        if (!bulletpool[i].active) return i;
    }

    return -1;
}

// Initialize all pool slots to inactive defaults.
void InitBulletPool(Bullet bulletpool[], int capacity)
{
    if (!bulletpool || capacity <= 0) return;

    for (int i = 0; i < capacity; ++i)
    {
        ResetBullet(&bulletpool[i]);
    }
}

// Spawn a new bullet if a free pool slot exists.
void FireBullet(Bullet bulletpool[], int capacity, Vector2 startPos, Vector2 direction, float speed, float size, Color color)
{
    int slot;

    if (!bulletpool || capacity <= 0) return;

    slot = FindInactiveBulletIndex(bulletpool, capacity);
    if (slot < 0) return;

    bulletpool[slot].pos = startPos;
    bulletpool[slot].dir = direction;
    bulletpool[slot].speed = speed;
    bulletpool[slot].size = size;
    bulletpool[slot].color = color;
    bulletpool[slot].active = true;
}

// Move active bullets and deactivate those beyond maximum travel radius.
void UpdateBulletPhysics(Bullet bulletpool[], int capacity, Vector2 playerPos)
{
    if (!bulletpool || capacity <= 0) return;

    for (int i = 0; i < capacity; ++i)
    {
        if (bulletpool[i].active)
        {
            // Integrate linear bullet motion.
            bulletpool[i].pos = Vector2Add(bulletpool[i].pos, Vector2Scale(bulletpool[i].dir, bulletpool[i].speed * GetFrameTime()));
            
            // Retire bullet if it outruns allowed lifetime range.
            float distance = Vector2DistanceSqr(bulletpool[i].pos, playerPos);
            if (distance >= BULLET_MAX_DISTANCE_SQ)
            {
                bulletpool[i].active = false;
            }
        }
    }
}

// Draw active bullets as filled circles.
void DrawBullet(Bullet bulletpool[], int capacity)
{
    if (!bulletpool || capacity <= 0) return;

    for (int i = 0; i < capacity; ++i)
    {
        if (bulletpool[i].active)
        {
            DrawCircleV(bulletpool[i].pos, bulletpool[i].size, bulletpool[i].color);
        }
    }
}