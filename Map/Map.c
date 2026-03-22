#include "Map.h"

GameMap InitRoom(void)
 {
    GameMap map;
    
    // Define the outer boundary
    map.bounds = (Rectangle){ 1000, 1000, 2400, 1800 }; 
    
    // Set colors
    map.FloorColor = DARKGRAY;
    map.WallColor = GRAY;

    //  Add some internal walls
    map.WallCount = 3;
    map.walls[0] = (Rectangle){ 1400, 1400, 200, 200 };  // A square pillar
    map.walls[1] = (Rectangle){ 2200, 1800, 400, 80 };  // A long horizontal wall
    map.walls[2] = (Rectangle){ 2800, 1400, 80, 600 };  // A vertical wall

    return map;
}

GameMap InitOpeningSmallRoom(void)
{
    GameMap map;

    // Compact intro room with one central obstacle to teach movement.
    map.bounds = (Rectangle){1000, 1000, 1300, 900};
    map.FloorColor = (Color){44, 49, 56, 255};
    map.WallColor = (Color){104, 112, 122, 255};

    map.WallCount = 1;
    map.walls[0] = (Rectangle){1500, 1320, 200, 260};

    return map;
}

GameMap InitOpeningBigRoom(void)
{
    GameMap map;

    // Larger staging room after opening door interaction.
    map.bounds = (Rectangle){1000, 900, 3000, 2100};
    map.FloorColor = (Color){34, 44, 52, 255};
    map.WallColor = (Color){96, 103, 112, 255};

    map.WallCount = 3;
    map.walls[0] = (Rectangle){1700, 1350, 220, 540};
    map.walls[1] = (Rectangle){2500, 1050, 520, 140};
    map.walls[2] = (Rectangle){3100, 1780, 180, 760};

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