#include "Player.h"
#include "raymath.h"
//Initialize player
void InitPlayer(Player* pl,Vector2 initpos)
{
    pl->pos=initpos;
    pl->speed=600.0f;
    pl->body=20.0f; // the size of the player
    pl->name="Player";
}
// Movement logic
void UpdatePlayerPos(Player* pl)
{
    pl->prevpos=pl->pos; // save the pos before moving
    pl->dir=(Vector2){0.0f,0.0f};
    // Keyboard input
    if (IsKeyDown(KEY_W)) pl->dir.y-=1.0f;
    if (IsKeyDown(KEY_S)) pl->dir.y+=1.0f;
    if (IsKeyDown(KEY_A)) pl->dir.x-=1.0f;
    if (IsKeyDown(KEY_D)) pl->dir.x+=1.0f;
    pl->dir=Vector2Normalize(pl->dir); // To ensure the player's speed is constant whatever the direction is
    pl->pos=Vector2Add(pl->pos,Vector2Scale(pl->dir,pl->speed*GetFrameTime()));
}
// Drawing the player
void DrawPlayer(Player* pl)
{
    DrawCircleV(pl->pos,pl->body,YELLOW);
    DrawText(pl->name,pl->pos.x-35,pl->pos.y-50,25,WHITE);
}