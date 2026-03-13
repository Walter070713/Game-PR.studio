#include "Enemy.h"
#include "Bullet.h"

// Initialize the enemypool
void InitEnemy(Enemy enemypool[],int emycapacity)
{
    for (int i=0;i<emycapacity;++i)
    {
        enemypool[i].flashtime=0.0f; // the time duration of the quick flash effect when the enemy get hit
        enemypool[i].pos=(Vector2){GetRandomValue(0,window_width),GetRandomValue(0,window_height)};
        enemypool[i].health=15;
        enemypool[i].active=true; // whether it's dead or alive
        enemypool[i].speed=400.0f;
        enemypool[i].body=30.0f; // enemy's size
        enemypool[i].state=WHITE; // the enemy turns red real quick and returns to white when it get hit
    }
}
// Update the enemy horde
void UpdateEnemyHorde(Enemy enemypool[],int emycapacity,Player* pl)
{
    // Loop through the horde so as to update ONLY alive enemies' position 
    for (int i=0;i<emycapacity;++i)
    {
        if (enemypool[i].active)
        {
            enemypool[i].pos=Vector2MoveTowards(enemypool[i].pos,pl->pos,enemypool[i].speed*GetFrameTime());
        }
        // Check if the enemy is in the RED state and making it return to normal WHITE state looply.
        if (enemypool[i].flashtime>0)
        {
            enemypool[i].flashtime-=GetFrameTime();
        }
    }
//spawn system
    float spawnTimer=0.0f;
    spawnTimer += GetFrameTime();
    if (spawnTimer >= 2.0f) { // Every 2 seconds an enemy comes to life
        for (int i = 0; i < emycapacity; ++i) {
            if (!enemypool[i].active) {
                enemypool[i].pos = (Vector2){ GetRandomValue(0, 2560), GetRandomValue(0, 1600) }; // Set position randomly
                enemypool[i].active = true;
                enemypool[i].health = 15;
                spawnTimer = 0; 
                break; // Only spawn one enemy per timer reset
            }
        }
    }

}
// void UpdateEnemyState(Enemy* enemy)
// {
//     if (enemy->flashtime>0)
//     {
//         enemy->flashtime-=GetFrameTime();
//     }
//     else
//     {
//         enemy->flashtime=0;
//     }
// }
// void UpdateEnemyPos(Enemy* enemy,Player* pl)
// {
    
    // if(CheckCollisionCircles(bullet->pos,bullet->size,enemy->pos,enemy->body))
    // {
    //     enemy->flashtime=0.1f;
    //     enemy->targetpos = Vector2Add(enemy->pos, Vector2Scale(bullet->dir, 50.0f));
    //     enemy->pos = Vector2Lerp(enemy->pos, enemy->targetpos, 0.2f);
    //     enemy->health-=1;
    //     bullet->hitenemy=true;
    // }
//     enemy->pos=Vector2MoveTowards(enemy->pos,pl->pos,enemy->speed*GetFrameTime());
// }