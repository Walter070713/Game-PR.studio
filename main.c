#include "raylib.h"
#include "raymath.h"
#include "window_setting.h"
#include "Player.h"
#include "CameraSet.h"
#include "MouseAim.h"
#include "MouseClicked.h"
#include "Weapon.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Collision.h"
#include "Spawn.h"
#include "Map.h"
#include "Weapon.h"
#include "GameStates.h"

// All the code done so far is coded by Walter from 6th Mar to 14th Mar
// Mostly from 18:30 to 24:00, sometimes to 3:00 am
// Weapon system on 17th Mar


// Basic window settings
const int window_width = 2560;
const int window_height = 1600;
Vector2 window_center = {(float)window_width/2,(float)window_height/2};

// Maximum possible bullet slots used by any weapon (pool size must be big enough to cover all weapons)
#define MAX_BULLET_POOL 60

int main(void) {
    const int emy_capacity = 5; // max enemy capacity

    // State Initialization
    GameState currentScreen = STATE_TITLE;

    // Game Objects
    Player plyr;
    Enemy enemypool[emy_capacity];
    float SpawnTimer = 0.0f;
    float SpawnRate = 2.0f;
    Bullet bulletpool[MAX_BULLET_POOL];
    MseAim mouse;
    Camera2D camera = {0};

    // Initializations
    GameMap room = InitRoom(); 
    InitPlayer(&plyr, window_center); 
    InitEnemy(enemypool, emy_capacity);
    InitCamera(&camera, window_center);
    InitBulletPool(bulletpool, GetWeaponBulletPoolSize(&plyr.weapon));
    InitWindow(window_width, window_height, "GAME by PR.studio");


    while (!WindowShouldClose()) 
    {
        switch (currentScreen) 
        {
            case STATE_TITLE:
                if (IsKeyPressed(KEY_ENTER)) currentScreen = STATE_GAMEPLAY;
                break;

            case STATE_SETTINGS:
                if (IsKeyPressed(KEY_ESCAPE)) currentScreen = STATE_TITLE;
                break;
            case STATE_GAMEPLAY:
                UpdateMouseAim(&mouse, camera, plyr.pos); // Logic to make player keep aiming at where the cursor is
                UpdatePlayerPos(&plyr); // Player movement logic
                UpdateWeapon(&plyr.weapon); // Update weapon timers and state
                UpdatePlayerStats(&plyr); // Update shield regen + hurt timer
                UpdateSpawner(enemypool, emy_capacity, plyr.pos, &SpawnTimer, SpawnRate, room); // Rebirth enemy
                UpdateEnemyHorde(enemypool, emy_capacity, plyr.pos); // Enemy movement logic
                UpdateBulletPhysics(bulletpool, GetWeaponBulletPoolSize(&plyr.weapon), plyr.pos); // Update bullet physics

                // Handle firing
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    if (FireWeapon(&plyr.weapon))
                    {
                        // Fire a bullet with weapon properties
                        FireBullet(bulletpool, GetWeaponBulletPoolSize(&plyr.weapon), plyr.pos, mouse.dir, 
                                plyr.weapon.bulletSpeed, plyr.weapon.bulletSize, plyr.weapon.bulletColor);
                    }
                }

                ResolveEnemyCollisions(&plyr, enemypool, emy_capacity, bulletpool, GetWeaponBulletPoolSize(&plyr.weapon));
                ResolveMapCollisions(&plyr, room, enemypool, emy_capacity, bulletpool, GetWeaponBulletPoolSize(&plyr.weapon));

                camera.target = Vector2Lerp(plyr.pos, camera.target, 0.001f); // To keep the player is always at center of the screen
                
                break;
        }

        BeginDrawing();
            ClearBackground(BLACK);
            switch (currentScreen) 
            {
                case STATE_TITLE:
                    DrawText("GAME-PR.STUDIO", 100, 100, 80, RED);
                    
                    if (IsOptionClicked("START MISSION", 100, 300, 40, WHITE, YELLOW)) 
                    {
                        currentScreen = STATE_GAMEPLAY;
                    }
                    if (IsOptionClicked("SETTINGS", 100, 380, 40, WHITE, YELLOW)) 
                    {
                        currentScreen = STATE_SETTINGS;
                    }
                    if (IsOptionClicked("EXIT", 100, 460, 40, WHITE, RED)) 
                    {
                        break; // Close logic handled by WindowShouldClose
                    }
                    break;

                case STATE_SETTINGS:
                    DrawText("SETTINGS", 100, 100, 60, YELLOW);
                    DrawText("- Use WASD to Move", 100, 200, 30, GRAY);
                    DrawText("- Mouse to Aim and Shoot", 100, 240, 30, GRAY);
                    DrawText("- R to Reload", 100, 280, 30, GRAY);
                    
                    if (IsOptionClicked("BACK TO MENU", 100, 500, 40, WHITE, GREEN)) 
                    {
                        currentScreen = STATE_TITLE;
                    }
                    break;
                case STATE_GAMEPLAY:
                    BeginMode2D(camera);
                    DrawMap(room); // room
                    DrawEnemy(enemypool, emy_capacity); // Enemy
                    DrawPlayer(&plyr); // Player
                    Vector2 WeaponEnd = Vector2Add(plyr.pos, Vector2Scale(mouse.dir, 50.0f)); // To form the player's weapon and make it point to the cursor
                    DrawLineEx(plyr.pos, WeaponEnd, 8.0f, RED); // Weapon
                    DrawBullet(bulletpool, GetWeaponBulletPoolSize(&plyr.weapon)); // Bullet
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

                    break;
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}