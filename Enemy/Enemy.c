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
    // enemy->pos=Vector2Lerp(enemy->pos,pl->pos,0.001f);
    enemy->pos=Vector2MoveTowards(enemy->pos,pl->pos,enemy->speed*GetFrameTime());
}