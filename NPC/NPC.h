#ifndef NPC_H
#define NPC_H

#include "raylib.h"
#include <stdbool.h>

typedef struct {
    Vector2 pos;
    const char* name;
    float body;
    Color color;
    bool active;
} NPC;

typedef struct {
    NPC npcs[16];
    int count;
} NPCPool;

void InitNPCPool(NPCPool* pool);
void AddNPC(NPCPool* pool, Vector2 pos, const char* name, float size, Color color);
void DrawNPCs(NPCPool* pool);

#endif
