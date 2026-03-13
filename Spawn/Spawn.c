#include "Spawn.h"
static const int spawnradius=800;
void UpdateSpawner(Enemy enemypool[], int emycpacity, Vector2 playerPos, float* spawnTimer, float spawnRate) 
{
    *spawnTimer += GetFrameTime();
    if (*spawnTimer >= spawnRate) 
    {
        *spawnTimer=0; // forced to wait for spawnrate seconds before trying again
        for (int i = 0; i < emycpacity; i++) 
        {
            if (!enemypool[i].active) 
            {
                // Logic: Spawn in a circle 800 pixels away from Player
                float angle = GetRandomValue(0, 360) * DEG2RAD;
                Vector2 offset = { cosf(angle) * spawnradius, sinf(angle) * spawnradius };
                enemypool[i].pos = Vector2Add(playerPos, offset);
                enemypool[i].active = true;
                enemypool[i].health = 10;
                enemypool[i].flashtime=0.0f;
                enemypool[i].state=WHITE;
                *spawnTimer = 0; // Reset the clock
                break; 
            }
        }
    }
}