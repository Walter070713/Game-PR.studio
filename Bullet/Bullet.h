#ifndef BULLET_H
#define BULLET_H
#include "raylib.h"
#include "raymath.h"
#include "Player.h"
#include "MouseAim.h"
typedef struct Bullet{
    Vector2 pos;
    Vector2 dir;
    Vector2 velocity;
    float speed;
    bool active;
}Bullet;
void InitBulletPool(Bullet bulletpool[],int capacity);
void UpdateBulletPos(Bullet bulletpool[],int capacity,Player* pl,MseAim* mouse);
void DrawBullet(Bullet bulletpool[],int capacity);
#endif