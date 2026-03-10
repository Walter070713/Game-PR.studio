#include "Collision.h"
void UpdateBulletLife(Bullet* bullet,Rectangle rec)
{
    if (CheckCollisionCircleRec(bullet->pos,bullet->size,rec))
    {
        bullet->active=false;
    }
}
void UpdateEnemyLife(Bullet* bullet,Enemy* enemy)
{
    if (CheckCollisionCircles(bullet->pos,bullet->size,enemy->pos,enemy->))
    {
        Color temp=enemy->state;
        for (int i=0;i<50;++i)
        {
            enemy->state=RED;
        }
        enemy->state=temp;
    }
}