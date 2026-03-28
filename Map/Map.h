#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_WALLS 50

// Runtime map descriptor (world bounds + optional legacy wall list).
typedef struct GameMap{
    Rectangle bounds;           // The room boundaries
    Rectangle walls[MAX_WALLS]; // Internal obstacles
    int WallCount;
    Color FloorColor;
    Color WallColor;
} GameMap;

// Build a basic map descriptor from bounds and colors.
GameMap CreateMapLayout(Rectangle bounds, Color floorColor, Color wallColor);

// Build active room from current TMX map.
GameMap InitRoom(void);

// Switch TMX map source used by room/collision systems.
bool SetActiveTMXMap(const char* mapPath);

// Recompute map bounds if display/runtime size changes.
void SyncMapToWindow(GameMap* map);

// Return active TMX tile size in world units.
float GetMapTileSize(void);

// Collision queries used by movement/combat systems.
bool IsMapCircleBlocked(Vector2 center, float radius, Rectangle mapBounds);
bool IsMapPointBlocked(Vector2 point, Rectangle mapBounds);

// Door detection and interaction helpers.
bool FindNearbyDoorInteractZone(Rectangle mapBounds, Vector2 fromPos, float maxDistance, Rectangle* outDoor);
const char* GetActiveTMXMapPath(void);

// Opened-door persistence helpers (used by save/load).
void ResetOpenedDoorZones(void);
int GetOpenedDoorZoneCount(void);
int GetOpenedDoorZones(Rectangle outZones[], int maxCount);
void SetOpenedDoorZones(const Rectangle zones[], int count);
void OpenDoorInteractZone(Rectangle doorZone);
bool IsDoorInteractZoneOpened(Rectangle doorZone);

// Render active map.
void DrawMap(GameMap map);

// Release TMX resources.
void ShutdownMapSystem(void);

#endif