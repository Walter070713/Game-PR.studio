#ifndef BULLET_H
#define BULLET_H
#include "raylib.h"
#include "raymath.h"
#include "Player.h"
#include "MouseAim.h"

// forward declaration for Enemy type used in function prototypes
typedef struct Enemy Enemy;

typedef struct Bullet{
    Vector2 pos;
    Vector2 dir;
    float size;
    float speed;
    bool active;
}Bullet;
void InitBulletPool(Bullet bulletpool[],int capacity);
void UpdateBulletPos(Bullet bulletpool[],int capacity,Player* pl,MseAim* mouse,Enemy* enemy);
void DrawBullet(Bullet bulletpool[],int capacity);
#endif