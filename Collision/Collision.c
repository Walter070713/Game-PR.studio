#include "Collision.h"

// Forward declarations (used before their definitions)
void ApplyDamageToPlayer(Player *pl, int damage);
void UpdatePlayerStats(Player *pl);

void ApplyDamageToPlayer(Player *pl, int damage)
{
    // Check I-Frames: If timer > 0, ignore damage
    if (pl->hurtTimer > 0) return;

    // Reset Shield Regen: Getting hit stops the recharge
    pl->shieldRegenTimer = 5.0f; // Must wait 5 seconds to regen again

    // Trigger I-Frames
    pl->hurtTimer = 0.6f; // Player is indamageable for 0.6 seconds

    // Damage to Shield first
    if (pl->shield > 0) {
        pl->shield -= damage;
        if (pl->shield < 0) {
            // Carry over excess damage to Health
            pl->health += pl->shield; 
            pl->shield = 0;
        }
    } else {
        // 5. Damage directly to Health
        pl->health -= damage;
    }

    // Check for Death
    if (pl->health <= 0) {
        pl->health = 0;
        // Trigger Game Over State here later
    }
}

void UpdatePlayerStats(Player *pl)
{

    // Tick down I-Frames
    if (pl->hurtTimer > 0) pl->hurtTimer -= GetFrameTime();

    // Handle Shield Regeneration
    if (pl->shieldRegenTimer > 0) 
    {
        pl->shieldRegenTimer -= GetFrameTime();
        pl->shieldRegenAccum = 0.0f; // Reset accumulator while waiting
    } 
    else 
    {
        // Regen shield slowly after the delay
        if (pl->shield < pl->maxshield) 
        {
            pl->shieldRegenAccum += GetFrameTime();
            if (pl->shieldRegenAccum >= 1.0f)  // 1 point per second
            { 
                pl->shield++;
                pl->shieldRegenAccum = 0.0f;
            }
        }
    }
}

void ResolveEnemyCollisions(Player* pl, Enemy e[], int eCount, Bullet b[], int bCount)
 {
    // Enemy involved part
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

                // Apply hit back effect
                float HitBackForce = 20.0f;
                e[i].pos = Vector2Add(e[i].pos, Vector2Scale(b[j].dir, HitBackForce));

                // Kill the bullet
                b[j].active = false;
            }
        }

        // ENEMY vs PLAYER
        if (CheckCollisionCircles(pl->pos, pl->body, e[i].pos, e[i].body)) 
        {
            ApplyDamageToPlayer(pl, 1);
            Vector2 push = Vector2Subtract(pl->pos, e[i].pos); // pushed direction
            push = Vector2Scale(Vector2Normalize(push), 1.0f); // push distance
            pl->pos = Vector2Add(pl->pos, push);
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

void ResolveMapCollisions(Player* pl, GameMap map,Enemy e[],int eCount,Bullet b[], int bCount)
 {
    // Inner walls collision
    for (int i = 0; i < map.WallCount; i++) 
    {
        // Player vs wall
        if (CheckCollisionCircleRec(pl->pos, pl->body, map.walls[i])) 
        {
            // Try only moving in X?
            Vector2 onlyX = { pl->pos.x, pl->prevpos.y };
            // Try only moving in Y?
            Vector2 onlyY = { pl->prevpos.x, pl->pos.y };

            if (!CheckCollisionCircleRec(onlyX, pl->body, map.walls[i])) 
            {
                pl->pos = onlyX; // Slide horizontally
            } 
            else if (!CheckCollisionCircleRec(onlyY, pl->body, map.walls[i]))
            {
                pl->pos = onlyY; // Slide vertically
            }
            else 
            {
                pl->pos = pl->prevpos; // Stuck in a corner, just stop
            }
        }

        // Enemy vs wall
        for (int j=0;j<eCount;++j)
        {
            if (CheckCollisionCircleRec(e[j].pos,e[j].body,map.walls[i]))
            {
            // Try only moving in X?
                Vector2 onlyX = { e[j].pos.x, e[j].prevpos.y };
            // Try only moving in Y?
                Vector2 onlyY = { e[j].prevpos.x, e[j].pos.y };
                if (!CheckCollisionCircleRec(onlyX, e[j].body, map.walls[i])) 
                {
                    e[j].pos = onlyX; // Slide horizontally
                } 
                else if (!CheckCollisionCircleRec(onlyY, e[j].body, map.walls[i]))
                {
                    e[j].pos = onlyY; // Slide vertically
                }
                else 
                {
                    e[j].pos = e[j].prevpos; // Stuck in a corner, just stop
                }
            }
        }

        // Bullet vs wall
        for (int k=0;k<bCount;++k)
        {
            if (b[k].active && CheckCollisionCircleRec(b[k].pos,b[k].size,map.walls[i]))
            {
                b[k].active=false;
            }
        }
    }
    // TMX tile solids collision (walls/furniture/doors/etc.)

    // Player
    if (IsMapCircleBlocked(pl->pos, pl->body * 0.88f, map.bounds))
    {
        Vector2 onlyX = { pl->pos.x, pl->prevpos.y };
        Vector2 onlyY = { pl->prevpos.x, pl->pos.y };

        if (!IsMapCircleBlocked(onlyX, pl->body * 0.88f, map.bounds))
        {
            pl->pos = onlyX;
        }
        else if (!IsMapCircleBlocked(onlyY, pl->body * 0.88f, map.bounds))
        {
            pl->pos = onlyY;
        }
        else
        {
            pl->pos = pl->prevpos;
        }
    }

    // Enemy
    for (int i = 0; i < eCount; ++i)
    {
        if (!e[i].active) continue;

        if (IsMapCircleBlocked(e[i].pos, e[i].body * 0.88f, map.bounds))
        {
            Vector2 onlyX = { e[i].pos.x, e[i].prevpos.y };
            Vector2 onlyY = { e[i].prevpos.x, e[i].pos.y };

            if (!IsMapCircleBlocked(onlyX, e[i].body * 0.88f, map.bounds))
            {
                e[i].pos = onlyX;
            }
            else if (!IsMapCircleBlocked(onlyY, e[i].body * 0.88f, map.bounds))
            {
                e[i].pos = onlyY;
            }
            else
            {
                e[i].pos = e[i].prevpos;
            }
        }
    }

    // Bullet
    for (int j = 0; j < bCount; ++j)
    {
        if (b[j].active && IsMapCircleBlocked(b[j].pos, b[j].size, map.bounds))
        {
            b[j].active = false;
        }
    }

    // Outer boundaries collision

    // Player
    // Keep X inside
    if (pl->pos.x < map.bounds.x + pl->body)  pl->pos.x = map.bounds.x + pl->body;
    if (pl->pos.x > map.bounds.x + map.bounds.width - pl->body)  pl->pos.x = map.bounds.x + map.bounds.width - pl->body;

    // Keep Y inside
    if (pl->pos.y < map.bounds.y + pl->body)  pl->pos.y = map.bounds.y + pl->body;
    if (pl->pos.y > map.bounds.y + map.bounds.height - pl->body) pl->pos.y = map.bounds.y + map.bounds.height - pl->body;

    // Enemy
    for (int i=0;i<eCount;++i)
    {
        // Keep X inside
        if (e[i].pos.x < map.bounds.x +e[i].body)  e[i].pos.x = map.bounds.x + e[i].body;
        if (e[i].pos.x > map.bounds.x + map.bounds.width - e[i].body)  e[i].pos.x = map.bounds.x + map.bounds.width - e[i].body;

        // Keep Y inside
        if (e[i].pos.y < map.bounds.y + e[i].body)  e[i].pos.y = map.bounds.y + e[i].body;
        if (e[i].pos.y > map.bounds.y + map.bounds.height - e[i].body) e[i].pos.y = map.bounds.y + map.bounds.height - e[i].body;
    }

    // Bullet vs boundary
    for (int j=0;j<bCount;++j)
    {
        if (b[j].active)
        {
            // Keep inside X
            if (b[j].pos.x < map.bounds.x + b[j].size || b[j].pos.x > map.bounds.x + map.bounds.width - b[j].size) 
                b[j].active = false;
            
            // Keep inside Y
            if (b[j].pos.y < map.bounds.y + b[j].size || b[j].pos.y > map.bounds.y + map.bounds.height - b[j].size) 
                b[j].active = false;
        }
    }
}