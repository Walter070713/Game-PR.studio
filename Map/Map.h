#ifndef MAP_H
#define MAP_H

#include "raylib.h"

#define MAX_WALLS 50

typedef struct GameMap{
    Rectangle bounds;           // The room boundaries
    Rectangle walls[MAX_WALLS]; // Internal obstacles
    int WallCount;
    Color FloorColor;
    Color WallColor;
} GameMap;

GameMap InitRoom(void);
void DrawMap(GameMap map);

#endif