#include "Player.h"
#include "raymath.h"

// Initialize player runtime state and default pistol loadout.
void InitPlayer(Player* pl, Vector2 initpos)
{
    if (!pl) return;

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

// Read WASD and update player position with normalized directional speed.
void UpdatePlayerPos(Player* pl)
{
    if (!pl) return;

    pl->prevpos = pl->pos; // save the pos before moving
    pl->dir = (Vector2){0.0f, 0.0f};
    
    // Keyboard input
    if (IsKeyDown(KEY_W)) pl->dir.y -= 1.0f;
    if (IsKeyDown(KEY_S)) pl->dir.y += 1.0f;
    if (IsKeyDown(KEY_A)) pl->dir.x -= 1.0f;
    if (IsKeyDown(KEY_D)) pl->dir.x += 1.0f;
    
    // Normalize to avoid diagonal speed boost.
    if (Vector2LengthSqr(pl->dir) > 0.0001f)
    {
        pl->dir = Vector2Normalize(pl->dir); // To ensure the player's speed is constant whatever the direction is
    }
    pl->pos = Vector2Add(pl->pos, Vector2Scale(pl->dir, pl->speed * GetFrameTime()));
}

// Draw player body plus name label centered above the character.
void DrawPlayer(Player* pl)
{
    const char* playerName;

    if (!pl) return;

    DrawCircleV(pl->pos, pl->body, YELLOW);

    playerName = (pl->name && pl->name[0] != '\0') ? pl->name : "Player";
    {
        float fontSize = pl->body * 1.6f;
        if (fontSize < 10.0f) fontSize = 10.0f;
        if (fontSize > 20.0f) fontSize = 20.0f;

        Vector2 textSize = MeasureTextEx(GetFontDefault(), playerName, fontSize, 1.0f);
        Vector2 textPos = {pl->pos.x - textSize.x * 0.5f, pl->pos.y - pl->body - fontSize - 3.0f};
        DrawTextEx(GetFontDefault(), playerName, textPos, fontSize, 1.0f, WHITE);
    }
}