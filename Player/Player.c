#include "Player.h"
#include "raymath.h"
void InitPlayer(Player* pl,Vector2 initpos)
{
    pl->pos=initpos;
    pl->speed=600.0f;
    pl->name="Player";
}
void UpdatePlayerPos(Player* pl)
{
    pl->dir=(Vector2){0.0f,0.0f};
    if (IsKeyDown(KEY_W)) pl->dir.y-=1.0f;
    if (IsKeyDown(KEY_S)) pl->dir.y+=1.0f;
    if (IsKeyDown(KEY_A)) pl->dir.x-=1.0f;
    if (IsKeyDown(KEY_D)) pl->dir.x+=1.0f;
    pl->dir=Vector2Normalize(pl->dir);
    pl->pos=Vector2Add(pl->pos,Vector2Scale(pl->dir,pl->speed*GetFrameTime()));
}
// void DrawPlayer(Player* pl)
// {
//     DrawCircleV(pl->pos,20.0f,YELLOW);
//     DrawText(pl->name,pl->pos.x-35,pl->pos.y-50,25,WHITE);
// }