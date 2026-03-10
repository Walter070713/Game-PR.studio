#ifndef BULLET_H
#define BULLET_H
#include "raylib.h"
#include "raymath.h"
#include "Player.h"
#include "MouseAim.h"
typedef struct Bullet{
    Vector2 pos;
    Vector2 dir;
    float speed;
    bool active;
}Bullet;
void InitBulletPool(Bullet bulletpool[],int capacity);
void UpdateBulletPool(Bullet bulletpool[],int capacity,Player* pl,MseAim* mouse);
#endif