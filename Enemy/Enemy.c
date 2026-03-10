#include "Enemy.h"
void InitEnemy(Enemy* enemy)
{
    enemy->flashtime=0.0f;
    enemy->pos=(Vector2){0,0};
    enemy->body=30.0f;
    enemy->state=WHITE;
}