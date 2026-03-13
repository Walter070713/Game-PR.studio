#include "raylib.h"
#include "raymath.h"
#include "window_setting.h"
#include "Player.h"
#include "CameraSet.h"
#include "MouseAim.h"
#include "Bullet.h"
#include "Enemy.h"
int main(void) {
    const int blt_capacity=20; // max bullet capacity
    Rectangle rec={window_center.x-500.0f,window_center.y-225.0f,1000.0f,500.0f};
    Player plyr;
    int emycapacity=5;
    Enemy enemypool[emycapacity];
    Bullet bulletpool[blt_capacity];
    MseAim mouse;
    Camera2D camera={0};
    InitPlayer(&plyr,window_center); // Initialize player
    InitEnemy(enemypool,emycapacity); // Initialize enemy
    InitCamera(&camera,window_center); // Initialize camera
    InitBulletPool(bulletpool,blt_capacity); // Initialize player's bullet state
    InitWindow(window_width, window_height, "GAME by PR.studio");
    while (!WindowShouldClose()) {
        UpdatePlayerPos(&plyr); // Player movement logic
        UpdateMouseAim(&mouse,camera,&plyr); // Logic to make player keep aiming at where the cursor is
        UpdateBulletPos(bulletpool,blt_capacity,&plyr,&mouse,&testenemy); // Bullet firing logic
        UpdateEnemyPos(&testenemy,&plyr); // Enemy movement logic
        UpdateEnemyState(&testenemy);
        camera.target=Vector2Lerp(plyr.pos,camera.target,0.001f); // To keep the player is always at center of the screen
        Vector2 WeaponEnd=Vector2Add(plyr.pos,Vector2Scale(mouse.dir,50.0f)); // To form the player's weapon and make it point to the cursor
        if (testenemy.flashtime>0.0f)
        {
            testenemy.state=RED;
        }
        else
        {
            testenemy.state=WHITE;
        }
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Health\nShield\nStrength",0,0,70,YELLOW);
            BeginMode2D(camera);
            DrawCircleV(testenemy.pos,testenemy.body,testenemy.state);
            DrawRectangle(2000,1000,800,400,WHITE);
            DrawRectangleLinesEx(rec,3.0f,WHITE);
            DrawPlayer(&plyr); // Player
            DrawLineEx(plyr.pos,WeaponEnd,8.0f,RED); // Weapon
            DrawBullet(bulletpool,blt_capacity); // Bullet
            EndMode2D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}