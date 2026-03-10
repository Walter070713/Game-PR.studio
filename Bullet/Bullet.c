#include "Bullet.h"
void InitBulletPool(Bullet bulletpool[],int capacity)
{
    for (int i=0;i<capacity;++i)
    {
        bulletpool[i].speed=50.0f;
        bulletpool[i].active=false;
    }
}
// void UpdateBulletPool(Bullet bulletpool[],int capacity)
// {

// }