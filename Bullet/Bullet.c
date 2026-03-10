#include "Bullet.h"
#include "Collision.h"
void InitBulletPool(Bullet bulletpool[],int capacity)
{
    for (int i=0;i<capacity;++i)
    {
        bulletpool[i].speed=1500.0f;
        bulletpool[i].size=12.0f;
        bulletpool[i].active=false;
    }
}
void UpdateBulletPos(Bullet bulletpool[],int capacity,Player* pl,MseAim* mouse)
{
    for(int i=0;i<capacity;++i)
    {
        if(bulletpool[i].active)
        {
            bulletpool[i].pos=Vector2Add(bulletpool[i].pos,Vector2Scale(bulletpool[i].velocity,GetFrameTime()));
            float distance=Vector2DistanceSqr(bulletpool[i].pos,pl->pos);
            if (distance>=2000000.0f)
            {
                bulletpool[i].active=false;
            }
            UpdateBulletLife(&bulletpool[i],(Rectangle){2000,1000,800,400,WHITE},&enemy);
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
                bulletpool[i].velocity=Vector2Scale(bulletpool[i].dir,bulletpool[i].speed);
                bulletpool[i].active=true;
                break;
            }
        }
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