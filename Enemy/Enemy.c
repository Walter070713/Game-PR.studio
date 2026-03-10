#include "Enemy.h"
void InitEnemy(Enemy* enemy)
{
    enemy->flashtime=0.0f;
    enemy->pos=(Vector2){0,0};
    enemy->speed=400.0f;
    enemy->body=30.0f;
    enemy->state=WHITE;
}
void UpdateEnemyPos(Enemy* enemy,Player* pl)
{
    enemy->dir=Vector2Subtract(pl->pos,enemy->pos);
    enemy->dir=Vector2Normalize(enemy->dir);
    enemy->pos=Vector2Add(enemy->pos,Vector2Scale(enemy->dir,enemy->speed*GetFrameTime()));
}