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
        enemy->targetpos = Vector2Add(enemy->pos, Vector2Scale(bullet->dir, 50.0f));
        enemy->pos = Vector2Lerp(enemy->pos, enemy->targetpos, 0.2f);
        bullet->active=false;
    }
}
void UpdateEnemyLife(Enemy* enemy)
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