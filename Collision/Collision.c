#include "Collision.h"
void UpdateBulletLife(Bullet* bullet,Rectangle rec,Enemy* enemy)
{
    if (CheckCollisionCircleRec(bullet->pos,bullet->size,rec))
    {
        bullet->active=false;
    }
    else if(CheckCollisionCircles(bullet->pos,bullet->size,enemy->pos,enemy->body))
    {
        enemy->flashtime=0.1f;
        bullet->active=false;
    }
}
void UpdateEnemyLife(Bullet* bullet,Enemy* enemy)
{
    if (CheckCollisionCircles(bullet->pos,bullet->size,enemy->pos,enemy->body))
    {
        if (enemy->flashtime>0)
        {
            enemy->flashtime-=GetFrameTime();
        }
    }
}