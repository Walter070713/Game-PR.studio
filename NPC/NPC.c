#include "NPC.h"

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
    
    for (int i = 0; i < pool->count; i++)
    {
        if (pool->npcs[i].active)
        {
            NPC* npc = &pool->npcs[i];
            
            // Draw circle
            DrawCircleV(npc->pos, npc->body, npc->color);
            
            // Draw name
            if (npc->name)
            {
                float fontSize = npc->body * 1.6f;
                if (fontSize < 10.0f) fontSize = 10.0f;
                if (fontSize > 20.0f) fontSize = 20.0f;
                
                Vector2 textSize = MeasureTextEx(GetFontDefault(), npc->name, fontSize, 1.0f);
                Vector2 textPos = {npc->pos.x - textSize.x * 0.5f, npc->pos.y - npc->body - fontSize - 3.0f};
                DrawTextEx(GetFontDefault(), npc->name, textPos, fontSize, 1.0f, WHITE);
            }
        }
    }
}
