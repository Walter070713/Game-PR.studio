#include "Player.h"
#include "raymath.h"

// Initialize player
void InitPlayer(Player* pl, Vector2 initpos)
{
    pl->pos = initpos;
    pl->prevpos = initpos;
    pl->speed = 600.0f;
    pl->body = 20.0f; // the size of the player
    pl->name = "Player";
    pl->health = 100;
    pl->shield = 50;
    pl->maxshield=50;
    pl->weapon = InitWeapon(WEAPON_PISTOL); // Initialize with pistol as default weapon
    pl->hurtTimer = 0.0f;
    pl->shieldRegenTimer = 0.0f;
    pl->shieldRegenAccum = 0.0f;
}

// Movement logic
void UpdatePlayerPos(Player* pl)
{
    pl->prevpos = pl->pos; // save the pos before moving
    pl->dir = (Vector2){0.0f, 0.0f};
    
    // Keyboard input
    if (IsKeyDown(KEY_W)) pl->dir.y -= 1.0f;
    if (IsKeyDown(KEY_S)) pl->dir.y += 1.0f;
    if (IsKeyDown(KEY_A)) pl->dir.x -= 1.0f;
    if (IsKeyDown(KEY_D)) pl->dir.x += 1.0f;
    
    pl->dir = Vector2Normalize(pl->dir); // To ensure the player's speed is constant whatever the direction is
    pl->pos = Vector2Add(pl->pos, Vector2Scale(pl->dir, pl->speed * GetFrameTime()));
}

// Drawing the player
void DrawPlayer(Player* pl)
{
    DrawCircleV(pl->pos, pl->body, YELLOW);
    {
        float fontSize = pl->body * 1.6f;
        if (fontSize < 10.0f) fontSize = 10.0f;
        if (fontSize > 20.0f) fontSize = 20.0f;

        Vector2 textSize = MeasureTextEx(GetFontDefault(), pl->name, fontSize, 1.0f);
        Vector2 textPos = {pl->pos.x - textSize.x * 0.5f, pl->pos.y - pl->body - fontSize - 3.0f};
        DrawTextEx(GetFontDefault(), pl->name, textPos, fontSize, 1.0f, WHITE);
    }
}