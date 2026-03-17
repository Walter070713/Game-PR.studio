#include "Spawn.h"
static const int spawnradius=800;

bool IsPointSafe(Vector2 point, float radius, Vector2 plpos, GameMap map) 
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

void UpdateSpawner(Enemy enemypool[], int emycpacity, Vector2 plpos, float* spawnTimer, float spawnRate, GameMap map) 
{
    *spawnTimer += GetFrameTime();
    if (*spawnTimer >= spawnRate) 
    {
        *spawnTimer=0; // forced to wait for spawnrate seconds before trying again
        for (int i = 0; i < emycpacity; i++) 
        {
            if (!enemypool[i].active) 
            {
                bool foundSafeSpot = false;
                Vector2 randompos;
                int attempts = 0;

                // Try to find a safe spot up to 30 times in this frame
                while (!foundSafeSpot && attempts < 30) 
                {
                    // Spawn enemies inside the room at random position
                    randompos.x = GetRandomValue(map.bounds.x, map.bounds.x + map.bounds.width);
                    randompos.y = GetRandomValue(map.bounds.y, map.bounds.y + map.bounds.height);

                    // Validate against the room and walls using your first function
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
                    *spawnTimer = 0; // Reset timer only if an enemy actually spawned
                    break; 
                }
            }
        }
    }
}

// void UpdateSpawner(Enemy enemypool[], int emycpacity, Vector2 playerPos, float* spawnTimer, float spawnRate) 
// {
//     *spawnTimer += GetFrameTime();
//     if (*spawnTimer >= spawnRate) 
//     {
//         *spawnTimer=0; // forced to wait for spawnrate seconds before trying again
//         for (int i = 0; i < emycpacity; i++) 
//         {
//             if (!enemypool[i].active) 
//             {
//                 // Logic: Spawn in a circle 800 pixels away from Player
//                 float angle = GetRandomValue(0, 360) * DEG2RAD;
//                 Vector2 offset = { cosf(angle) * spawnradius, sinf(angle) * spawnradius };
//                 enemypool[i].pos = Vector2Add(playerPos, offset);
//                 enemypool[i].active = true;
//                 enemypool[i].health = 10;
//                 enemypool[i].flashtime=0.0f;
//                 enemypool[i].state=WHITE;
//                 *spawnTimer = 0; // Reset the clock
//                 break; 
//             }
//         }
//     }
// }