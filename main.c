#include "raylib.h"
#include "raymath.h"
#include "Player.h"
int main(void) {
    const int window_width=2560;
    const int window_height=1600;
    Vector2 window_center={(float)window_width/2,(float)window_height/2};
    Player plyr;
    InitPlayer(&plyr,window_center);
    InitWindow(window_width, window_height, "GAME by PR.studio");
    while (!WindowShouldClose()) {
        UpdatePlayerPos(&plyr);
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Health\nShield\nStrength",0,0,70,YELLOW);
            DrawPlayer(&plyr);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}