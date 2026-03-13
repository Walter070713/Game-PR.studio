#include "Enemy.h"
#include "Bullet.h"
void InitEnemy(Enemy enemypool[],int emycapacity)
{
    for (int i=0;i<emycapacity;++i)
    {
        enemypool[i].flashtime=0.0f;
        enemypool[i].pos=(Vector2){GetRandomValue(0,),0};
        enemypool[i].health=15;
        enemypool[i].speed=400.0f;
        enemypool[i].body=30.0f;
        enemypool[i].state=WHITE;
    }
}
void UpdateEnemyState(Enemy* enemy)
{
    if (enemy->flashtime>0)
    {
        enemy->flashtime-=GetFrameTime();
    }
    else
    {
        enemy->flashtime=0;
    }
}
void UpdateEnemyPos(Enemy* enemy,Player* pl)
{
    
    // if(CheckCollisionCircles(bullet->pos,bullet->size,enemy->pos,enemy->body))
    // {
    //     enemy->flashtime=0.1f;
    //     enemy->targetpos = Vector2Add(enemy->pos, Vector2Scale(bullet->dir, 50.0f));
    //     enemy->pos = Vector2Lerp(enemy->pos, enemy->targetpos, 0.2f);
    //     enemy->health-=1;
    //     bullet->hitenemy=true;
    // }
    enemy->pos=Vector2MoveTowards(enemy->pos,pl->pos,enemy->speed*GetFrameTime());
}