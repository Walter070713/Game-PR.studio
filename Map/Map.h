#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_WALLS 50

typedef struct GameMap{
    Rectangle bounds;           // The room boundaries
    Rectangle walls[MAX_WALLS]; // Internal obstacles
    int WallCount;
    Color FloorColor;
    Color WallColor;
} GameMap;

GameMap CreateMapLayout(Rectangle bounds, Color floorColor, Color wallColor);
bool AddMapWall(GameMap* map, Rectangle wall);

GameMap InitRoom(void);
bool SetActiveTMXMap(const char* mapPath);
void SyncMapToWindow(GameMap* map);
float GetMapTileSize(void);
bool IsMapCircleBlocked(Vector2 center, float radius, Rectangle mapBounds);
bool IsMapPointBlocked(Vector2 point, Rectangle mapBounds);
bool FindPrimaryDoorInteractZone(Rectangle mapBounds, Rectangle* outDoor);
bool FindNearbyDoorInteractZone(Rectangle mapBounds, Vector2 fromPos, float maxDistance, Rectangle* outDoor);
void OpenDoorInteractZone(Rectangle doorZone);
bool HasOpenedDoor(void);
bool IsDoorInteractZoneOpened(Rectangle doorZone);
void DrawMap(GameMap map);
void ShutdownMapSystem(void);

#endif