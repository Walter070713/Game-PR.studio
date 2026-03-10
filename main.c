#include "raylib.h"
#include "raymath.h"
#include "Player.h"
int main(void) {
    const int window_width=2560;
    const int window_height=1600;
    Vector2 window_center={(float)window_width/2,(float)window_height/2};
    Player plyr;
    Camera2D camera={0};
    InitPlayer(&plyr,window_center);
    InitWindow(window_width, window_height, "GAME by PR.studio");
    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}