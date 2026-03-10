#ifndef BULLET_H
#define BULLET_H
#include "raylib.h"
typedef struct Bullet{
    Vector2 pos;
    Vector2 dir;
    float speed;
    bool active;
}Bullet;
void InitBulletPool(Bullet bulletpool[],int capacity);
#endif