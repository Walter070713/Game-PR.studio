#include "Map.h"

// Generic map constructor used by all chapter/level room presets.
GameMap CreateMapLayout(Rectangle bounds, Color floorColor, Color wallColor)
{
    GameMap map = {0};
    map.bounds = bounds;
    map.FloorColor = floorColor;
    map.WallColor = wallColor;
    map.WallCount = 0;
    return map;
}

// Adds one wall safely; returns false when MAX_WALLS is reached.
bool AddMapWall(GameMap* map, Rectangle wall)
{
    if (!map || map->WallCount >= MAX_WALLS) return false;
    map->walls[map->WallCount++] = wall;
    return true;
}

GameMap InitRoom(void)
 {
    // Level-combat default arena.
    GameMap map = CreateMapLayout((Rectangle){1000, 1000, 2400, 1800}, DARKGRAY, GRAY);

    AddMapWall(&map, (Rectangle){1400, 1400, 200, 200});
    AddMapWall(&map, (Rectangle){2200, 1800, 400, 80});
    AddMapWall(&map, (Rectangle){2800, 1400, 80, 600});

    return map;
}

GameMap InitOpeningSmallRoom(void)
{
    // Opening room 1: compact traversal and interaction space.
    GameMap map = CreateMapLayout(
        (Rectangle){1000, 1000, 1300, 900},
        (Color){44, 49, 56, 255},
        (Color){104, 112, 122, 255}
    );

    AddMapWall(&map, (Rectangle){1500, 1320, 200, 260});

    return map;
}

GameMap InitOpeningBigRoom(void)
{
    // Opening room 2: larger peaceful area before tutorial handoff.
    GameMap map = CreateMapLayout(
        (Rectangle){1000, 900, 3000, 2100},
        (Color){34, 44, 52, 255},
        (Color){96, 103, 112, 255}
    );

    AddMapWall(&map, (Rectangle){1700, 1350, 220, 540});
    AddMapWall(&map, (Rectangle){2500, 1050, 520, 140});
    AddMapWall(&map, (Rectangle){3100, 1780, 180, 760});

    return map;
}

GameMap InitTutorialRoom(void)
{
    // Tutorial room: simple pathing plus one terminal objective zone.
    GameMap map = CreateMapLayout(
        (Rectangle){950, 950, 2400, 1700},
        (Color){39, 44, 58, 255},
        (Color){112, 118, 132, 255}
    );

    AddMapWall(&map, (Rectangle){1450, 1180, 220, 500});
    AddMapWall(&map, (Rectangle){2050, 2000, 680, 120});
    AddMapWall(&map, (Rectangle){2800, 1300, 160, 700});

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