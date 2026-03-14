#include "Map.h"

GameMap InitRoom(void)
 {
    GameMap map;
    
    // Define the outer boundary
    map.bounds = (Rectangle){ 0, 0, 2400, 1800 }; 
    
    // Set colors
    map.FloorColor = DARKGRAY;
    map.WallColor = GRAY;

    //  Add some internal walls
    map.WallCount = 3;
    map.walls[0] = (Rectangle){ 400, 400, 200, 200 };  // A square pillar
    map.walls[1] = (Rectangle){ 1200, 800, 400, 80 };  // A long horizontal wall
    map.walls[2] = (Rectangle){ 1800, 400, 80, 600 };  // A vertical wall

    return map;
}

void DrawMap(GameMap map) 
{
    // Draw the floor
    DrawRectangleRec(map.bounds, map.FloorColor);
    
    // Draw the internal walls
    for (int i = 0; i < map.WallCount; i++) 
    {
        DrawRectangleRec(map.walls[i], map.WallColor);
        //Draw a thin line around the wall
        DrawRectangleLinesEx(map.walls[i], 2.0f, BLACK);
    }

    // Draw the outer boundary lines
    DrawRectangleLinesEx(map.bounds, 15.0f, MAROON);
}