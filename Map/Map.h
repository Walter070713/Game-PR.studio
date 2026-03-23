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
GameMap InitOpeningSmallRoom(void);
GameMap InitOpeningBigRoom(void);
GameMap InitTutorialRoom(void);
void DrawMap(GameMap map);

#endif