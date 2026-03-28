#include "Spawn.h"

// Validate random spawn candidates against bounds/walls/player distance.
static bool IsPointSafe(Vector2 point, float radius, Vector2 plpos, GameMap map)
{
    // Check if it's outside the main boundary
    if (point.x - radius < map.bounds.x || 
        point.x + radius > map.bounds.x + map.bounds.width ||
        point.y - radius < map.bounds.y || 
        point.y + radius > map.bounds.y + map.bounds.height) {
        return false; 
    }

    // Check if it's inside any internal walls
    for (int i = 0; i < map.WallCount; ++i) 
    {
        if (CheckCollisionCircleRec(point, radius, map.walls[i])) return false;
    }

    // Check if it's too close to the Player
    if (Vector2DistanceSqr(point, plpos) < 90000.0f) return false;

    // Point is safe
    return true; 
}

// Time-gated enemy spawner that activates the first available inactive enemy slot.
void UpdateSpawner(Enemy enemypool[], int emycpacity, Vector2 plpos, float* spawnTimer, float spawnRate, GameMap map)
{
    if (!enemypool || emycpacity <= 0 || !spawnTimer || spawnRate <= 0.0f) return;

    *spawnTimer += GetFrameTime();
    if (*spawnTimer >= spawnRate)
    {
        *spawnTimer = 0.0f; // forced to wait for spawnrate seconds before trying again
        for (int i = 0; i < emycpacity; i++)
        {
            if (!enemypool[i].active)
            {
                bool foundSafeSpot = false;
                Vector2 randompos;
                int attempts = 0;

                // Try up to 30 candidates to avoid expensive unbounded searching.
                while (!foundSafeSpot && attempts < 30)
                {
                    // Spawn enemies inside the room at random position
                    randompos.x = (float)GetRandomValue((int)map.bounds.x, (int)(map.bounds.x + map.bounds.width));
                    randompos.y = (float)GetRandomValue((int)map.bounds.y, (int)(map.bounds.y + map.bounds.height));

                    // Candidate must pass all safety constraints.
                    if (IsPointSafe(randompos, enemypool[i].body, plpos, map))
                    {
                        foundSafeSpot = true;
                    }
                    attempts++;
                }

                // Only activate the enemy if the spot is safe
                if (foundSafeSpot)
                {
                    enemypool[i].pos = randompos;
                    enemypool[i].active = true;
                    enemypool[i].health = 3;
                    enemypool[i].flashtime = 0.0f;
                    enemypool[i].state = WHITE;
                    break;
                }
            }
        }
    }
}
