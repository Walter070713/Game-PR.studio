#include "Collision.h"

// Normalize vector, but return fallback when magnitude is too small.
static Vector2 NormalizeOrFallback(Vector2 v, Vector2 fallback)
{
    if (Vector2LengthSqr(v) <= 0.0001f) return fallback;
    return Vector2Normalize(v);
}

// Axis-separated slide resolver used after map collision detection.
static Vector2 ResolveCircleSlide(Vector2 currentPos, Vector2 previousPos, float radius, Rectangle mapBounds)
{
    Vector2 onlyX = {currentPos.x, previousPos.y};
    Vector2 onlyY = {previousPos.x, currentPos.y};

    if (!IsMapCircleBlocked(onlyX, radius, mapBounds)) return onlyX;
    if (!IsMapCircleBlocked(onlyY, radius, mapBounds)) return onlyY;
    return previousPos;
}

// Keep circle center inside map bounds while respecting radius.
static void ClampCircleToBounds(Vector2* pos, float radius, Rectangle bounds)
{
    if (!pos) return;

    if (pos->x < bounds.x + radius) pos->x = bounds.x + radius;
    if (pos->x > bounds.x + bounds.width - radius) pos->x = bounds.x + bounds.width - radius;
    if (pos->y < bounds.y + radius) pos->y = bounds.y + radius;
    if (pos->y > bounds.y + bounds.height - radius) pos->y = bounds.y + bounds.height - radius;
}

// Boundary check helper for bullet lifetime against world rectangle.
static bool IsBulletOutOfBounds(const Bullet* bullet, Rectangle bounds)
{
    if (!bullet) return true;

    if (bullet->pos.x < bounds.x + bullet->size || bullet->pos.x > bounds.x + bounds.width - bullet->size) return true;
    if (bullet->pos.y < bounds.y + bullet->size || bullet->pos.y > bounds.y + bounds.height - bullet->size) return true;
    return false;
}

// Damage pipeline: apply I-frames, shield/health damage, and regen reset.
static void ApplyDamageToPlayer(Player *pl, int damage)
{
    if (!pl || damage <= 0) return;

    // Ignore hit while player is still invulnerable.
    if (pl->hurtTimer > 0) return;

    // Any incoming hit pauses shield regeneration.
    pl->shieldRegenTimer = 5.0f; // Must wait 5 seconds to regen again

    // Trigger temporary invulnerability window.
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
        // Shield depleted: route damage directly to health.
        pl->health -= damage;
    }

    // Check for Death
    if (pl->health <= 0) {
        pl->health = 0;
        // Game-over transition is handled elsewhere.
    }
}

    // Update post-damage timers: I-frames and shield regeneration.
void UpdatePlayerStats(Player *pl)
{
    if (!pl) return;

    // Tick down I-Frames
    if (pl->hurtTimer > 0) pl->hurtTimer -= GetFrameTime();

    // Handle delayed shield regeneration.
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

// Resolve combat collisions among enemies, bullets, and player.
void ResolveEnemyCollisions(Player* pl, Enemy e[], int eCount, Bullet b[], int bCount)
{
    if (!pl || !e || eCount <= 0) return;

    // Iterate active enemies as primary collision participants.
    for (int i = 0; i < eCount; i++) 
    {
        if (!e[i].active) continue;

        // BULLET vs ENEMY
        for (int j = 0; b && j < bCount; j++)
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
            Vector2 push = NormalizeOrFallback(Vector2Subtract(pl->pos, e[i].pos), (Vector2){1.0f, 0.0f});
            pl->pos = Vector2Add(pl->pos, push);
            e[i].pos = Vector2Subtract(e[i].pos, push);
        }

        // ENEMY vs ENEMY 
        for (int k = i + 1; k < eCount; k++)
        {
            if (e[k].active && CheckCollisionCircles(e[i].pos, e[i].body, e[k].pos, e[k].body)) 
            {
                // push them apart
                Vector2 push = NormalizeOrFallback(Vector2Subtract(e[i].pos, e[k].pos), (Vector2){1.0f, 0.0f});
                e[i].pos = Vector2Add(e[i].pos, push);
                e[k].pos = Vector2Subtract(e[k].pos, push);
            }
        }
    }
}

// Resolve collisions against TMX solids and hard map bounds.
void ResolveMapCollisions(Player* pl, GameMap map,Enemy e[],int eCount,Bullet b[], int bCount)
{
    if (!pl) return;

    float playerRadius = pl->body * 0.88f;

    // Player vs TMX collision map with axis slide fallback.
    if (IsMapCircleBlocked(pl->pos, playerRadius, map.bounds))
    {
        pl->pos = ResolveCircleSlide(pl->pos, pl->prevpos, playerRadius, map.bounds);
    }

    // Enemy vs TMX collision map with axis slide fallback.
    for (int i = 0; e && i < eCount; ++i)
    {
        float enemyRadius;
        if (!e[i].active) continue;

        enemyRadius = e[i].body * 0.88f;
        if (IsMapCircleBlocked(e[i].pos, enemyRadius, map.bounds))
        {
            e[i].pos = ResolveCircleSlide(e[i].pos, e[i].prevpos, enemyRadius, map.bounds);
        }
    }

    // Bullets are destroyed when they hit blocked TMX cells.
    for (int j = 0; b && j < bCount; ++j)
    {
        if (b[j].active && IsMapCircleBlocked(b[j].pos, b[j].size, map.bounds))
        {
            b[j].active = false;
        }
    }

    // Keep player inside map bounds.
    ClampCircleToBounds(&pl->pos, pl->body, map.bounds);

    // Keep active enemies inside world bounds.
    for (int i = 0; e && i < eCount; ++i)
    {
        if (!e[i].active) continue;
        ClampCircleToBounds(&e[i].pos, e[i].body, map.bounds);
    }

    // Bullets are destroyed when they leave world bounds.
    for (int j = 0; b && j < bCount; ++j)
    {
        if (!b[j].active) continue;
        if (IsBulletOutOfBounds(&b[j], map.bounds)) b[j].active = false;
    }
}