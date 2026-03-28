#ifndef NPC_H
#define NPC_H

#include "raylib.h"
#include <stdbool.h>

typedef struct Player Player;

// Lightweight NPC unit used in exploration phases.
typedef struct {
    Vector2 pos;
    const char* name;
    float body;
    Color color;
    bool active;
} NPC;

// Fixed-size NPC pool for current room.
typedef struct {
    NPC npcs[16];
    int count;
} NPCPool;

// Reset all NPC slots to inactive.
void InitNPCPool(NPCPool* pool);

// Add one NPC if pool has capacity.
void AddNPC(NPCPool* pool, Vector2 pos, const char* name, float size, Color color);

// Draw active NPCs and labels.
void DrawNPCs(NPCPool* pool);

// Push player out of NPC collision bodies.
void ResolvePlayerNPCCollision(Player* player, const NPCPool* pool);

#endif
