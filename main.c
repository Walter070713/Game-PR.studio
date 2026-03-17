#include "raylib.h"
#include "raymath.h"
#include "window_setting.h"
#include "Player.h"
#include "CameraSet.h"
#include "MouseAim.h"
#include "Weapon.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Collision.h"
#include "Spawn.h"
#include "Map.h"
#include "Weapon.h"

// All the code done so far is coded by Walter from 6th Mar to 14th Mar
// Mostly from 18:30 to 24:00, sometimes to 3:00 am
// Weapon system refactored on 17th Mar


// Basic window settings
const int window_width = 2560;
const int window_height = 1600;
Vector2 window_center = {(float)window_width/2,(float)window_height/2};

int main(void) {
    const int blt_capacity = 50; // max bullet capacity (increased for more flexibility)
    const int emy_capacity = 5; // max enemy capacity

    Player plyr;
    Enemy enemypool[emy_capacity];
    float SpawnTimer = 0.0f;
    float SpawnRate = 2.0f;
    Bullet bulletpool[blt_capacity];
    MseAim mouse;
    Camera2D camera = {0};

    GameMap room = InitRoom(); // Initialize the room
    InitPlayer(&plyr, window_center); // Initialize player and weapon unit
    InitEnemy(enemypool, emy_capacity); // Initialize enemy
    InitCamera(&camera, window_center); // Initialize camera
    InitBulletPool(bulletpool, blt_capacity); // Initialize bullet pool
    InitWindow(window_width, window_height, "GAME by PR.studio");

    while (!WindowShouldClose()) 
    {
        UpdateMouseAim(&mouse, camera, plyr.pos); // Logic to make player keep aiming at where the cursor is
        UpdatePlayerPos(&plyr); // Player movement logic
        UpdateWeapon(&plyr.weapon); // Update weapon timers and state
        UpdatePlayerStats(&plyr); // Update shield regen + hurt timer
        UpdateSpawner(enemypool, emy_capacity, plyr.pos, &SpawnTimer, SpawnRate, room); // Rebirth enemy
        UpdateEnemyHorde(enemypool, emy_capacity, plyr.pos); // Enemy movement logic
        UpdateBulletPhysics(bulletpool, blt_capacity, plyr.pos); // Update bullet physics

        // Handle firing
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (FireWeapon(&plyr.weapon))
            {
                // Fire a bullet with weapon properties
                FireBullet(bulletpool, blt_capacity, plyr.pos, mouse.dir, 
                          plyr.weapon.bulletSpeed, plyr.weapon.bulletSize, plyr.weapon.bulletColor);
            }
        }

        ResolveEnemyCollisions(&plyr, enemypool, emy_capacity, bulletpool, blt_capacity);
        ResolveMapCollisions(&plyr, room, enemypool, emy_capacity, bulletpool, blt_capacity);

        camera.target = Vector2Lerp(plyr.pos, camera.target, 0.001f); // To keep the player is always at center of the screen
        Vector2 WeaponEnd = Vector2Add(plyr.pos, Vector2Scale(mouse.dir, 50.0f)); // To form the player's weapon and make it point to the cursor
        
        BeginDrawing();
            ClearBackground(BLACK);
            
            BeginMode2D(camera);
            DrawMap(room); // room
            DrawEnemy(enemypool, emy_capacity); // Enemy
            DrawPlayer(&plyr); // Player
            DrawLineEx(plyr.pos, WeaponEnd, 8.0f, RED); // Weapon
            DrawBullet(bulletpool, blt_capacity); // Bullet
            EndMode2D();
            
            // Draw HUD - Health and weapon status (after world rendering so it's on top)
            DrawText(plyr.name, 10, 10, 40, YELLOW);
            DrawText(TextFormat("Health: %d", plyr.health), 10, 60, 30, RED);
            DrawText(TextFormat("Shield: %d", plyr.shield), 10, 100, 30, BLUE);
            
            // Draw weapon info
            WeaponInfo winfo = GetWeaponInfo(&plyr.weapon);
            DrawText(TextFormat("Magazine: %d/%d", winfo.magazine, plyr.weapon.maxMagazine), 10, 150, 30, YELLOW);
            DrawText(TextFormat("Total Ammo: %d", winfo.totalAmmo), 10, 190, 30, YELLOW);
            
            if (winfo.isReloading)
            {
                DrawText("RELOADING", 10, 230, 30, ORANGE);
                DrawRectangle(10, 270, (int)(200 * winfo.reloadProgress), 20, ORANGE);
                DrawRectangleLines(10, 270, 200, 20, WHITE);
            }
            else if (winfo.magazine == 0)
            {
                DrawText("PRESS R TO RELOAD", 10, 230, 25, RED);
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}