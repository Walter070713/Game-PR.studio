#include "Bullet.h"

// Initialize the bullet pool
void InitBulletPool(Bullet bulletpool[], int capacity)
{
    for (int i = 0; i < capacity; ++i)
    {
        bulletpool[i].active = false;
        bulletpool[i].speed = 0.0f;
        bulletpool[i].size = 0.0f;
        bulletpool[i].pos = (Vector2){0.0f, 0.0f};
        bulletpool[i].dir = (Vector2){0.0f, 0.0f};
        bulletpool[i].color = WHITE;
    }
}

// Fire a bullet from player position with weapon properties
void FireBullet(Bullet bulletpool[], int capacity, Vector2 startPos, Vector2 direction, float speed, float size, Color color)
{
    for (int i = 0; i < capacity; ++i)
    {
        if (!bulletpool[i].active)
        {
            bulletpool[i].pos = startPos;
            bulletpool[i].dir = direction;
            bulletpool[i].speed = speed;
            bulletpool[i].size = size;
            bulletpool[i].color = color;
            bulletpool[i].active = true;
            break;
        }
    }
}

// Update bullet physics (movement and lifetime)
void UpdateBulletPhysics(Bullet bulletpool[], int capacity, Vector2 playerPos)
{
    for (int i = 0; i < capacity; ++i)
    {
        if (bulletpool[i].active)
        {
            // Update position based on direction and speed
            bulletpool[i].pos = Vector2Add(bulletpool[i].pos, Vector2Scale(bulletpool[i].dir, bulletpool[i].speed * GetFrameTime()));
            
            // Deactivate if too far from player
            float distance = Vector2DistanceSqr(bulletpool[i].pos, playerPos);
            if (distance >= 2000000.0f)
            {
                bulletpool[i].active = false;
            }
        }
    }
}

// Draw all active bullets
void DrawBullet(Bullet bulletpool[], int capacity)
{
    for (int i = 0; i < capacity; ++i)
    {
        if (bulletpool[i].active)
        {
            DrawCircleV(bulletpool[i].pos, bulletpool[i].size, bulletpool[i].color);
        }
    }
}