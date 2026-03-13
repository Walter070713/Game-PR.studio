#include "Bullet.h"
#include "Enemy.h"
void InitBulletPool(Bullet bulletpool[],int capacity)
{
    for (int i=0;i<capacity;++i)
    {
        bulletpool[i].speed=1500.0f;
        bulletpool[i].size=12.0f;
        bulletpool[i].active=false;
    }
}
void UpdateBulletPos(Bullet bulletpool[],int capacity,Player* pl,MseAim* mouse,Enemy* enemy)
{
    for(int i=0;i<capacity;++i)
    {
        if(bulletpool[i].active)
        {
            bulletpool[i].pos=Vector2Add(bulletpool[i].pos,Vector2Scale(bulletpool[i].dir,bulletpool[i].speed*GetFrameTime()));
            float distance=Vector2DistanceSqr(bulletpool[i].pos,pl->pos);
            if (distance>=2000000.0f)
            {
                bulletpool[i].active=false;
            }
        UpdateBulletLife(&bulletpool[i],(Rectangle){2000,1000,800,400},enemy);
        }
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        for (int i=0;i<capacity;++i)
        {
            if(!bulletpool[i].active)
            {
                bulletpool[i].pos=pl->pos;
                bulletpool[i].dir=mouse->dir;
                bulletpool[i].active=true;
                break;
            }
        }
    }
}
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
        enemy->health-=1;
        bullet->active=false;
    }
}
void DrawBullet(Bullet bulletpool[],int capacity)
{
    for(int i=0;i<capacity;++i)
    {
        if (bulletpool[i].active)
        {
            DrawCircleV(bulletpool[i].pos,bulletpool[i].size,YELLOW);
        }
    }
}