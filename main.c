#include "raylib.h"
#include "raymath.h"
#include "Player.h"
#include "CameraSet.h"
#include "MouseAim.h"
#include "Bullet.h"
#include "Collision.h"
int main(void) {
    const int window_width=2560;
    const int window_height=1600;
    const int capacity=20;
    Vector2 window_center={(float)window_width/2,(float)window_height/2};
    Rectangle rec={window_center.x-500.0f,window_center.y-225.0f,1000.0f,500.0f};
    Player plyr;
    Bullet bulletpool[capacity];
    MseAim mouse;
    Camera2D camera={0};
    InitPlayer(&plyr,window_center);
    InitCamera(&camera,window_center);
    InitBulletPool(bulletpool,capacity);
    InitWindow(window_width, window_height, "GAME by PR.studio");
    while (!WindowShouldClose()) {
        UpdatePlayerPos(&plyr);
        UpdateMouseAim(&mouse,camera,&plyr);
        UpdateBulletPos(bulletpool,capacity,&plyr,&mouse);
        camera.target=Vector2Lerp(plyr.pos,camera.target,0.1f);
        Vector2 WeaponEnd=Vector2Add(plyr.pos,Vector2Scale(mouse.dir,50.0f));
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Health\nShield\nStrength",0,0,70,YELLOW);
            BeginMode2D(camera);
            DrawRectangle(2000,1000,800,400,WHITE);
            DrawRectangleLinesEx(rec,3.0f,WHITE);
            DrawPlayer(&plyr);
            DrawLineEx(plyr.pos,WeaponEnd,8.0f,RED);
            DrawBullet(bulletpool,capacity);
            EndMode2D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}