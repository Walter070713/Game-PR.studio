#include "Collision.h"
void UpdateBulletLife(Bullet* bullet,Rectangle rec,Enemy* enemy)
{
    if (CheckCollisionCircleRec(bullet->pos,bullet->size,rec)||CheckCollisionCircles(bullet->pos,bullet->size,enemy->pos,enemy->body))
    {
        bullet->active=false;
    }
}
void UpdateEnemyLife(Bullet* bullet,Enemy* enemy)
{
    if (CheckCollisionCircles(bullet->pos,bullet->size,enemy->pos,enemy->body))
    {
        Color temp=enemy->state;
        for (int i=0;i<100;++i)
        {
            enemy->state=RED;
        }
        enemy->state=temp;
    }
}