#include "raylib.h"
#include "raymath.h"
#include "window_setting.h"
#include "Player.h"
#include "CameraSet.h"
#include "MouseAim.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Collision.h"
#include "Spawn.h"

// Basic window settings
const int window_width = 2560;
const int window_height = 1600;
Vector2 window_center = {(float)window_width/2,(float)window_height/2};

int main(void) {
    const int blt_capacity=20; // max bullet capacity
    const int emy_capacity=5; // max enemy capacity
    Rectangle rec={window_center.x-500.0f,window_center.y-225.0f,1000.0f,500.0f};
    Player plyr;
    Enemy enemypool[emy_capacity];
    float SpawnTimer=0.0f;
    float SpawnRate=2.0f;
    Bullet bulletpool[blt_capacity];
    MseAim mouse;
    Camera2D camera={0};
    InitPlayer(&plyr,window_center); // Initialize player
    InitEnemy(enemypool,emy_capacity); // Initialize enemy
    InitCamera(&camera,window_center); // Initialize camera
    InitBulletPool(bulletpool,blt_capacity); // Initialize player's bullet state
    InitWindow(window_width, window_height, "GAME by PR.studio");

    while (!WindowShouldClose()) {
        UpdateMouseAim(&mouse,camera,plyr.pos); // Logic to make player keep aiming at where the cursor is
        UpdatePlayerPos(&plyr); // Player movement logic
        UpdateSpawner(enemypool, emy_capacity,plyr.pos, &SpawnTimer,SpawnRate); // Rebirth enemy
        UpdateEnemyHorde(enemypool,emy_capacity,plyr.pos); // Enemy movement logic
        UpdateBulletPos(bulletpool,blt_capacity,plyr.pos,mouse.dir); // Bullet firing logic
        ResolveAllCollisions(&plyr,enemypool,emy_capacity, bulletpool,blt_capacity);
        camera.target=Vector2Lerp(plyr.pos,camera.target,0.001f); // To keep the player is always at center of the screen
        Vector2 WeaponEnd=Vector2Add(plyr.pos,Vector2Scale(mouse.dir,50.0f)); // To form the player's weapon and make it point to the cursor
        
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Health\nShield\nStrength",0,0,70,YELLOW);
            BeginMode2D(camera);
            DrawRectangle(2000,1000,800,400,WHITE);
            DrawRectangleLinesEx(rec,3.0f,WHITE);
            DrawEnemy(enemypool,emy_capacity); // Enemy
            DrawPlayer(&plyr); // Player
            DrawLineEx(plyr.pos,WeaponEnd,8.0f,RED); // Weapon
            DrawBullet(bulletpool,blt_capacity); // Bullet
            EndMode2D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}