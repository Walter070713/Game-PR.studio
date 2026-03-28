#include "NPC.h"
#include "Player.h"
#include "raymath.h"

// Cached optional texture for Rondy world rendering.
static Texture2D gRondyTexture = {0};
static bool gTriedLoadRondyTexture = false;
static bool gRondyTextureLoaded = false;
// Silent Rondy sprite has large transparent padding; boost render size so visible body matches player size.
static const float kRondyVisualCompensation = 5.0f;
// Keep gameplay collision sane even when visual sprite is oversized.
static const float kRondyCollisionCompensation = 1.35f;

// Compute visual radius (can differ from collision radius for stylized sprites).
static float GetNPCVisualRadius(const NPC* npc)
{
    if (!npc) return 0.0f;
    if (npc->name && TextIsEqual(npc->name, "Rondy")) return npc->body * kRondyVisualCompensation;
    return npc->body;
}

// Compute collision radius used for player push-out.
static float GetNPCCollisionRadius(const NPC* npc)
{
    if (!npc) return 0.0f;
    if (npc->name && TextIsEqual(npc->name, "Rondy")) return npc->body * kRondyCollisionCompensation;
    return npc->body;
}

// Lazy-load Rondy sprite once and reuse it for subsequent frames.
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

// Reset NPC pool state.
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

// Append one NPC to the active pool.
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

// Draw active NPCs with sprite fallback and name labels.
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

            // Draw Rondy as sprite when available, otherwise draw basic circle body.
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

// Keep player from overlapping active NPC collision bodies.
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
            // Push player outward along separation vector.
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
