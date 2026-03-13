#include "Collision.h"
void CheckBulletEnemyCollisions(Bullet b[], int bCount, Enemy e[], int eCount) 
{
    for (int i = 0; i < bCount; ++i) 
    {
        if (!b[i].active) continue; // continue the loop if bullet is not fired
        for (int j = 0; j < eCount; j++) 
        {
            if (!e[j].active) continue; // continue if enemy is already dead
            if (CheckCollisionCircles(b[i].pos, b[i].size, e[j].pos, e[j].body)) 
            {
                // Apply damage and flash
                e[j].health -= 1;
                e[j].flashtime = 0.1f;
                // Kill the bullet
                b[i].active = false;
                break; // Stop checking other enemies for this already fired bullet
            }
        }
    }
}

// void CheckBulletBlockCollisions(Bullet b[], int bCount)
// {

// }