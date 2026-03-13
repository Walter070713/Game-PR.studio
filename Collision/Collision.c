#include "Collision.h"

void ResolveAllCollisions(Player* pl, Enemy e[], int eCount, Bullet b[], int bCount)
 {
    
    for (int i = 0; i < eCount; i++) 
    {
        if (!e[i].active) continue;

        // BULLET vs ENEMY
        for (int j = 0; j < bCount; j++)
         {
            if (b[j].active && CheckCollisionCircles(b[j].pos, b[j].size, e[i].pos, e[i].body))
             {
                // Apply damage and flash
                e[i].health -= 1;
                e[i].flashtime = 0.1f;
                b[j].active = false;
            }
        }

        // ENEMY vs PLAYER
        if (CheckCollisionCircles(pl->pos, pl->body, e[i].pos, e[i].body)) 
        {
            pl->health -= 1; // Direct damage
            // pushed by enemies
            Vector2 push = Vector2Subtract(pl->pos,e[i].pos);
            push = Vector2Scale(Vector2Normalize(push), 1.0f);
            pl->pos=Vector2Add(pl->pos, push);
            e[i].pos = Vector2Subtract(e[i].pos, push);
        }

        // ENEMY vs ENEMY 
        for (int k = i + 1; k < eCount; k++)
        {
            if (e[k].active && CheckCollisionCircles(e[i].pos, e[i].body, e[k].pos, e[k].body)) 
            {
                // push them apart
                Vector2 push = Vector2Subtract(e[i].pos, e[k].pos); // direction
                push = Vector2Scale(Vector2Normalize(push), 1.0f); // push distance
                e[i].pos = Vector2Add(e[i].pos, push);
                e[k].pos = Vector2Subtract(e[k].pos, push);
            }
        }
    }
}
