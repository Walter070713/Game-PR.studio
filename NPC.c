#include "NPC.h"
#include "Player.h"
#include "raymath.h"

static Texture2D gRondyTexture = {0};
static bool gTriedLoadRondyTexture = false;
static bool gRondyTextureLoaded = false;
// Silent Rondy sprite has large transparent padding; boost render size so visible body matches player size.
static const float kRondyVisualCompensation = 5.0f;
// Keep gameplay collision sane even when visual sprite is oversized.
static const float kRondyCollisionCompensation = 1.35f;

static float GetNPCVisualRadius(const NPC* npc)
{
    if (!npc) return 0.0f;
    if (npc->name && TextIsEqual(npc->name, "Rondy")) return npc->body * kRondyVisualCompensation;
    return npc->body;
}

static float GetNPCCollisionRadius(const NPC* npc)
{
    if (!npc) return 0.0f;
    if (npc->name && TextIsEqual(npc->name, "Rondy")) return npc->body * kRondyCollisionCompensation;
    return npc->body;
}

static void EnsureRondyTextureLoaded(void)
{
    if (gTriedLoadRondyTexture) return;
    gTriedLoadRondyTexture = true;

    const char* path = "Assets/characters/Rondy/Silent Rondy.png";
    if (FileExists(path))
    {
        gRondyTexture = LoadTexture(path);
        gRondyTextureLoaded = (gRondyTexture.id != 0);
    }
}

void InitNPCPool(NPCPool* pool)
{
    if (!pool) return;
    pool->count = 0;
    for (int i = 0; i < 16; i++)
    {
        pool->npcs[i].active = false;
        pool->npcs[i].pos = (Vector2){0, 0};
        pool->npcs[i].name = 0;
        pool->npcs[i].body = 0;
        pool->npcs[i].color = WHITE;
    }
}

void AddNPC(NPCPool* pool, Vector2 pos, const char* name, float size, Color color)
{
    if (!pool || pool->count >= 16) return;
    
    NPC* npc = &pool->npcs[pool->count];
    npc->pos = pos;
    npc->name = name;
    npc->body = size;
    npc->color = color;
    npc->active = true;
    pool->count++;
}

void DrawNPCs(NPCPool* pool)
{
    if (!pool) return;

    EnsureRondyTextureLoaded();
    
    for (int i = 0; i < pool->count; i++)
    {
        if (pool->npcs[i].active)
        {
            NPC* npc = &pool->npcs[i];
            float visualRadius = GetNPCVisualRadius(npc);

            // Render Rondy sprite if available, otherwise fallback to circle.
            if (npc->name && TextIsEqual(npc->name, "Rondy") && gRondyTextureLoaded)
            {
                float targetDiameter = visualRadius * 2.0f;
                Rectangle src = {0, 0, (float)gRondyTexture.width, (float)gRondyTexture.height};
                Rectangle dst = {
                    npc->pos.x - targetDiameter * 0.5f,
                    npc->pos.y - targetDiameter * 0.5f,
                    targetDiameter,
                    targetDiameter
                };
                DrawTexturePro(gRondyTexture, src, dst, (Vector2){0}, 0.0f, WHITE);
            }
            else
            {
                DrawCircleV(npc->pos, npc->body, npc->color);
            }
            
            // Draw name
            if (npc->name)
            {
                float collisionRadius = GetNPCCollisionRadius(npc);
                float fontSize = npc->body * 1.2f;
                if (fontSize < 10.0f) fontSize = 10.0f;
                if (fontSize > 24.0f) fontSize = 24.0f;
                
                Vector2 textSize = MeasureTextEx(GetFontDefault(), npc->name, fontSize, 1.0f);
                Vector2 textPos = {
                    npc->pos.x - textSize.x * 0.5f,
                    npc->pos.y - collisionRadius - fontSize - 4.0f
                };
                DrawTextEx(GetFontDefault(), npc->name, textPos, fontSize, 1.0f, WHITE);
            }
        }
    }
}

void ResolvePlayerNPCCollision(Player* player, const NPCPool* pool)
{
    if (!player || !pool) return;

    for (int i = 0; i < pool->count; i++)
    {
        const NPC* npc = &pool->npcs[i];
        if (!npc->active) continue;

        Vector2 delta = Vector2Subtract(player->pos, npc->pos);
        float distance = Vector2Length(delta);
        float minDistance = player->body + GetNPCCollisionRadius(npc);

        if (distance < minDistance)
        {
            Vector2 direction;
            if (distance <= 0.0001f)
            {
                direction = (Vector2){1.0f, 0.0f};
                distance = 0.0001f;
            }
            else
            {
                direction = Vector2Scale(delta, 1.0f / distance);
            }

            float pushOut = (minDistance - distance);
            player->pos = Vector2Add(player->pos, Vector2Scale(direction, pushOut));
        }
    }
}
