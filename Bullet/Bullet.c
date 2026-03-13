#include "Bullet.h"

// Initialize the bullet
void InitBulletPool(Bullet bulletpool[],int capacity)
{
    for (int i=0;i<capacity;++i)
    {
        bulletpool[i].speed=1500.0f;
        bulletpool[i].size=12.0f;
        bulletpool[i].active=false; // whether it's fired
    }
}
void UpdateBulletPos(Bullet bulletpool[],int capacity,Vector2 plpos,Vector2 mousedir)
{
    for(int i=0;i<capacity;++i)
    {
        if(bulletpool[i].active)
        {
            bulletpool[i].pos=Vector2Add(bulletpool[i].pos,Vector2Scale(bulletpool[i].dir,bulletpool[i].speed*GetFrameTime()));
            float distance=Vector2DistanceSqr(bulletpool[i].pos,plpos);
            if (distance>=2000000.0f) // bullet reach the maximum distance will return to the player
            {
                bulletpool[i].active=false;
            }
        }
    }
    // Mouse input logic to fire
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        for (int i=0;i<capacity;++i)
        {
            if(!bulletpool[i].active)
            {
                bulletpool[i].pos=plpos;
                bulletpool[i].dir=mousedir;
                bulletpool[i].active=true;
                break;
            }
        }
    }
}

// Draw fired bulllet
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