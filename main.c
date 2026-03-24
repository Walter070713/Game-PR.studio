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
#include "GameStates.h"
#include "Scene.h"
#include "OpeningPhase.h"
#include "TutorialPhase.h"
#include "NPC.h"
#include <string.h>

// Main architecture:
// - STATE_TITLE / STATE_SETTINGS / STATE_SCENE are UI-driven states.
// - STATE_GAMEPLAY runs TMX map exploration with door interaction.


// Basic window settings
int window_width = 2560;
int window_height = 1600;
bool isFullscreen = false;
Vector2 window_center;

// Maximum possible bullet slots used by any weapon
#define MAX_BULLET_POOL 60

static void StartOpeningMission(OpeningFlow* openingFlow, Scene* currentScene, GameState* currentScreen, GameMap* room, Player* player,
    Bullet bulletPool[], int bulletPoolSize, Enemy enemyPool[], int enemyCapacity, float* spawnTimer, NPCPool* npcpool)
{
    // Start opening scene first, then map gameplay after scene completion.
    OpeningStartMission(openingFlow, currentScene, room, player, bulletPool, bulletPoolSize, enemyPool, enemyCapacity, spawnTimer, npcpool);
    *currentScreen = STATE_SCENE;
}

static void CaptureNameInput(char* buffer, int maxLen)
{
    int key = GetCharPressed();
    while (key > 0)
    {
        int len = (int)strlen(buffer);
        if (key >= 32 && key <= 126 && len < maxLen - 1)
        {
            buffer[len] = (char)key;
            buffer[len + 1] = '\0';
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE))
    {
        int len = (int)strlen(buffer);
        if (len > 0) buffer[len - 1] = '\0';
    }
}

static void ApplyDynamicCameraAndMovement(Player* player, GameMap* room, Camera2D* camera)
{
    if (!player || !room || !camera) return;

    SyncMapToWindow(room);

    if (room->bounds.width > 0.0f && room->bounds.height > 0.0f)
    {
        float zoomX = (float)GetScreenWidth() / room->bounds.width;
        float zoomY = (float)GetScreenHeight() / room->bounds.height;
        float fitZoom = (zoomX < zoomY) ? zoomX : zoomY;
        float targetZoom = fitZoom * 2.6f;
        float lerpFactor = GetFrameTime() * 8.0f;

        if (targetZoom < 0.8f) targetZoom = 0.8f;
        if (targetZoom > 7.0f) targetZoom = 7.0f;
        if (lerpFactor > 1.0f) lerpFactor = 1.0f;

        if (camera->zoom <= 0.0f) camera->zoom = targetZoom;
        camera->zoom = Lerp(camera->zoom, targetZoom, lerpFactor);
    }

    camera->offset = (Vector2){(float)GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.5f};
    camera->target = player->pos;

    // Human-sized feel against furniture: player diameter ~0.62 tile.
    player->body = GetMapTileSize() * 0.31f;

    // Tile-relative base movement speed; keep it controlled after zoom changes.
    {
        float worldSpeed = GetMapTileSize() * 6.0f; // ~6 tiles/second
        if (worldSpeed < 64.0f) worldSpeed = 64.0f;
        if (worldSpeed > 240.0f) worldSpeed = 240.0f;
        player->speed = worldSpeed;
    }
}

int main(void) {
    const int emy_capacity = 5; // max enemy capacity

    // Initialize window center
    window_center = (Vector2){(float)window_width/2, (float)window_height/2};

    // State Initialization
    // Start from title; progression flows: Title → Opening Scene → Map → Level
    GameState currentScreen = STATE_TITLE;

    // Scene system
    Scene currentScene = {0};
    OpeningFlow openingFlow = {0};
    TutorialFlow tutorialFlow = {0};
    char playerName[24] = "";
    bool showExitConfirm = false;
    bool showCreditsPanel = false;
    bool requestExit = false;
    Texture2D titleBackground = {0};
    bool titleBackgroundLoaded = false;

    // Game Objects
    Player plyr;
    Enemy enemypool[emy_capacity];
    NPCPool npcpool;
    float SpawnTimer = 0.0f;
    Bullet bulletpool[MAX_BULLET_POOL];
    Camera2D camera = {0};

    InitWindow(window_width, window_height, "EXECUTE");

    // Initializations
    GameMap room = InitRoom(); 
    InitPlayer(&plyr, window_center); 
    InitEnemy(enemypool, emy_capacity);
    InitNPCPool(&npcpool);
    InitCamera(&camera, window_center);
    InitBulletPool(bulletpool, GetWeaponBulletPoolSize(&plyr.weapon));
    InitOpeningFlow(&openingFlow);
    InitTutorialFlow(&tutorialFlow);
    ApplyDynamicCameraAndMovement(&plyr, &room, &camera);

    if (FileExists("Assets/backgrounds/2-MC IN SERVER ROOM.png"))
    {
        titleBackground = LoadTexture("Assets/backgrounds/2-MC IN SERVER ROOM.png");
        titleBackgroundLoaded = (titleBackground.id != 0);
    }

    // Spawn player inside the active map bounds for TMX visual testing.
    plyr.pos = (Vector2){room.bounds.x + room.bounds.width * 0.5f, room.bounds.y + room.bounds.height * 0.5f};
    plyr.prevpos = plyr.pos;


    while (!WindowShouldClose() && !requestExit)
    {
        // Global input handling
        if (IsKeyPressed(KEY_F11))  ToggleFullscreenMode();
        
        switch (currentScreen) 
        {
            case STATE_TITLE:
                if (!showExitConfirm && IsKeyPressed(KEY_ENTER)) {
                    playerName[0] = '\0';
                    currentScreen = STATE_NAME_ENTRY;
                }
                if ((showExitConfirm || showCreditsPanel) && IsKeyPressed(KEY_ESCAPE))
                {
                    showExitConfirm = false;
                    showCreditsPanel = false;
                }
                break;

            case STATE_NAME_ENTRY:
                CaptureNameInput(playerName, (int)sizeof(playerName));
                if (IsKeyPressed(KEY_ENTER))
                {
                    if (playerName[0] == '\0')
                    {
                        strcpy(playerName, "Player");
                    }
                    plyr.name = playerName;
                    StartOpeningMission(&openingFlow, &currentScene, &currentScreen, &room, &plyr, bulletpool, GetWeaponBulletPoolSize(&plyr.weapon), enemypool, emy_capacity, &SpawnTimer, &npcpool);
                }
                if (IsKeyPressed(KEY_ESCAPE))
                {
                    currentScreen = STATE_TITLE;
                }
                break;

            case STATE_SETTINGS:
                if (IsKeyPressed(KEY_ESCAPE)) currentScreen = STATE_TITLE;
                break;
            case STATE_GAMEPLAY: {
                ApplyDynamicCameraAndMovement(&plyr, &room, &camera);

                // Keep NPC body size aligned with player body after dynamic scaling.
                for (int i = 0; i < npcpool.count; i++)
                {
                    if (npcpool.npcs[i].active) npcpool.npcs[i].body = plyr.body;
                }

                UpdatePlayerPos(&plyr);

                if (tutorialFlow.isActive)
                {
                    UpdateTutorialFlow(&tutorialFlow, &plyr, &room, enemypool, emy_capacity, bulletpool, GetWeaponBulletPoolSize(&plyr.weapon), camera);
                }
                else
                {
                    bool shouldEnterScene = false;
                    bool shouldStartTutorial = false;
                    UpdateOpeningPeacefulPhase(
                        &openingFlow,
                        &plyr,
                        &room,
                        enemypool,
                        emy_capacity,
                        bulletpool,
                        &SpawnTimer,
                        &currentScene,
                        &npcpool,
                        &shouldEnterScene,
                        &shouldStartTutorial
                    );

                    if (shouldEnterScene)
                    {
                        currentScreen = STATE_SCENE;
                        break;
                    }

                    if (shouldStartTutorial)
                    {
                        StartTutorialFlow(&tutorialFlow, &room, &plyr, bulletpool, GetWeaponBulletPoolSize(&plyr.weapon), enemypool, emy_capacity, &SpawnTimer);
                        InitNPCPool(&npcpool);
                    }
                }

                ResolvePlayerNPCCollision(&plyr, &npcpool);

                // Keep the player pinned to screen center every frame.
                camera.target = plyr.pos;

                break;
            }

            case STATE_SCENE:
                // Update scene state
                if (!UpdateScene(&currentScene)) {
                    OpeningHandleSceneComplete(
                        &openingFlow,
                        &room,
                        &plyr,
                        bulletpool,
                        GetWeaponBulletPoolSize(&plyr.weapon),
                        enemypool,
                        emy_capacity,
                        &SpawnTimer,
                        &npcpool
                    );

                    // Scene finished, return to gameplay
                    currentScreen = STATE_GAMEPLAY;
                }
                break;
        }

        // Render pass follows current game state.
        BeginDrawing();
            ClearBackground(BLACK);
            
            // Always draw the scene if active
            if (currentScreen == STATE_SCENE) {
                DrawScene(&currentScene);
            }
            
            switch (currentScreen) 
            {
                case STATE_TITLE:
                {
                    if (titleBackgroundLoaded)
                    {
                        float scrollX = sinf((float)GetTime() * 0.36f) * 52.0f;
                        float scrollY = cosf((float)GetTime() * 0.28f) * 12.0f;
                        float zoom = 1.14f;
                        float dw = (float)GetScreenWidth() * zoom;
                        float dh = (float)GetScreenHeight() * zoom;
                        float dx = -((dw - (float)GetScreenWidth()) * 0.5f) + scrollX;
                        float dy = -((dh - (float)GetScreenHeight()) * 0.5f) + scrollY;
                        Rectangle src = {0, 0, (float)titleBackground.width, (float)titleBackground.height};
                        Rectangle dst = {dx, dy, dw, dh};
                        DrawTexturePro(titleBackground, src, dst, (Vector2){0}, 0.0f, WHITE);
                    }

                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){8, 12, 18, 120});
                    DrawRectangle(64, 58, 890, (int)((float)GetScreenHeight() - 116.0f), (Color){12, 18, 28, 178});
                    DrawRectangleLines(64, 58, 890, (int)((float)GetScreenHeight() - 116.0f), (Color){240, 202, 90, 230});

                    DrawText("EXECUTE", 108, 114, 132, (Color){246, 233, 180, 255});

                    if (!showExitConfirm && !showCreditsPanel)
                    {
                        if (IsOptionClicked("NEW GAME", 124, 360, 54, WHITE, YELLOW))
                        {
                            playerName[0] = '\0';
                            currentScreen = STATE_NAME_ENTRY;
                        }
                        if (IsOptionClicked("SETTING", 124, 446, 54, WHITE, YELLOW))
                        {
                            currentScreen = STATE_SETTINGS;
                        }
                        if (IsOptionClicked("CREDITS", 124, 532, 54, WHITE, YELLOW))
                        {
                            showCreditsPanel = true;
                        }
                        if (IsOptionClicked("EXIT", 124, 618, 54, WHITE, RED))
                        {
                            showExitConfirm = true;
                        }
                    }

                    if (showCreditsPanel)
                    {
                        int sw = GetScreenWidth();
                        int sh = GetScreenHeight();
                        Rectangle modal = {(float)sw * 0.5f - 320.0f, (float)sh * 0.5f - 180.0f, 640.0f, 360.0f};
                        DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 165});
                        DrawRectangleRec(modal, (Color){30, 34, 44, 242});
                        DrawRectangleLinesEx(modal, 3.0f, (Color){240, 202, 90, 255});
                        DrawText("CREDITS", (int)modal.x + 220, (int)modal.y + 36, 56, (Color){250, 240, 205, 255});
                        DrawText("Game & Art Direction : PR.studio", (int)modal.x + 70, (int)modal.y + 132, 30, LIGHTGRAY);
                        DrawText("Powered by Raylib", (int)modal.x + 180, (int)modal.y + 175, 30, LIGHTGRAY);
                        if (IsOptionClicked("CLOSE", (int)modal.x + 248, (int)modal.y + 262, 38, WHITE, GREEN))
                        {
                            showCreditsPanel = false;
                        }
                    }

                    if (showExitConfirm)
                    {
                        int sw = GetScreenWidth();
                        int sh = GetScreenHeight();
                        Rectangle modal = {(float)sw * 0.5f - 280.0f, (float)sh * 0.5f - 130.0f, 560.0f, 260.0f};
                        DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 160});
                        DrawRectangleRec(modal, (Color){30, 34, 44, 240});
                        DrawRectangleLinesEx(modal, 3.0f, (Color){240, 202, 90, 255});
                        DrawText("Exit EXECUTE?", (int)modal.x + 150, (int)modal.y + 48, 44, (Color){250, 240, 205, 255});
                        DrawText("This will close the game.", (int)modal.x + 130, (int)modal.y + 108, 28, LIGHTGRAY);

                        if (IsOptionClicked("YES", (int)modal.x + 120, (int)modal.y + 176, 36, WHITE, RED))
                        {
                            requestExit = true;
                        }
                        if (IsOptionClicked("NO", (int)modal.x + 330, (int)modal.y + 176, 36, WHITE, GREEN))
                        {
                            showExitConfirm = false;
                        }
                    }
                    break;
                }

                case STATE_NAME_ENTRY:
                {
                    DrawText("ENTER YOUR NAME", 100, 120, 72, YELLOW);
                    DrawText("This name will be used for your character.", 100, 220, 30, LIGHTGRAY);

                    Rectangle inputBox = {100, 300, 820, 90};
                    DrawRectangleRec(inputBox, (Color){30, 30, 30, 220});
                    DrawRectangleLinesEx(inputBox, 3.0f, WHITE);

                    const char* shownName = (playerName[0] != '\0') ? playerName : "Type your name...";
                    Color shownColor = (playerName[0] != '\0') ? WHITE : GRAY;
                    DrawText(shownName, (int)inputBox.x + 20, (int)inputBox.y + 28, 36, shownColor);

                    DrawText("ENTER: Confirm  |  BACKSPACE: Delete  |  ESC: Back", 100, 420, 26, LIGHTGRAY);

                    bool canStart = playerName[0] != '\0';
                    Color startColor = canStart ? GREEN : DARKGRAY;
                    if (IsOptionClicked("BEGIN", 100, 500, 42, startColor, YELLOW))
                    {
                        if (playerName[0] == '\0') strcpy(playerName, "Player");
                        plyr.name = playerName;
                        StartOpeningMission(&openingFlow, &currentScene, &currentScreen, &room, &plyr, bulletpool, GetWeaponBulletPoolSize(&plyr.weapon), enemypool, emy_capacity, &SpawnTimer, &npcpool);
                    }

                    break;
                }

                case STATE_SETTINGS:
                {
                    if (titleBackgroundLoaded)
                    {
                        float scrollX = sinf((float)GetTime() * 0.36f) * 52.0f;
                        float scrollY = cosf((float)GetTime() * 0.28f) * 12.0f;
                        float zoom = 1.14f;
                        float dw = (float)GetScreenWidth() * zoom;
                        float dh = (float)GetScreenHeight() * zoom;
                        float dx = -((dw - (float)GetScreenWidth()) * 0.5f) + scrollX;
                        float dy = -((dh - (float)GetScreenHeight()) * 0.5f) + scrollY;
                        Rectangle src = {0, 0, (float)titleBackground.width, (float)titleBackground.height};
                        Rectangle dst = {dx, dy, dw, dh};
                        DrawTexturePro(titleBackground, src, dst, (Vector2){0}, 0.0f, WHITE);
                    }

                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){8, 12, 18, 145});
                    DrawRectangle(82, 80, 980, 760, (Color){14, 20, 32, 180});
                    DrawRectangleLines(82, 80, 980, 760, (Color){240, 202, 90, 230});

                    DrawText("SETTINGS", 120, 120, 82, (Color){246, 233, 180, 255});
                    DrawText("CONTROLS", 124, 238, 38, (Color){200, 226, 255, 255});
                    DrawText("WASD : Move", 130, 294, 30, (Color){236, 240, 248, 255});
                    DrawText("Mouse : Aim and Shoot", 130, 336, 30, (Color){236, 240, 248, 255});
                    DrawText("R : Reload", 130, 378, 30, (Color){236, 240, 248, 255});
                    DrawText("F11 : Toggle Fullscreen", 130, 420, 30, (Color){236, 240, 248, 255});

                    DrawText("RESOLUTION", 124, 490, 38, (Color){200, 226, 255, 255});
                    if (IsOptionClicked("2560 x 1600", 140, 545, 32, WHITE, YELLOW))
                    {
                        if (isFullscreen) ToggleFullscreenMode();
                        ChangeResolution(2560, 1600, &camera);
                    }
                    if (IsOptionClicked("3200 x 2000", 140, 590, 32, WHITE, YELLOW))
                    {
                        if (isFullscreen) ToggleFullscreenMode();
                        ChangeResolution(3200, 2000, &camera);
                    }
                    if (IsOptionClicked("1920 x 1080", 140, 635, 32, WHITE, YELLOW))
                    {
                        if (isFullscreen) ToggleFullscreenMode();
                        ChangeResolution(1920, 1080, &camera);
                    }

                    char* fullscreenText = isFullscreen ? "WINDOWED MODE" : "FULLSCREEN MODE";
                    if (IsOptionClicked(fullscreenText, 140, 694, 32, WHITE, YELLOW)) ToggleFullscreenMode();
                    if (IsOptionClicked("BACK TO MENU", 124, 762, 38, WHITE, GREEN)) currentScreen = STATE_TITLE;

                    break;
                }
                case STATE_GAMEPLAY:
                {
                    BeginMode2D(camera);
                    DrawMap(room);
                    if (tutorialFlow.isActive)
                    {
                        DrawTutorialWorldOverlay(&tutorialFlow, &plyr, enemypool, emy_capacity, bulletpool, GetWeaponBulletPoolSize(&plyr.weapon), camera);
                    }
                    else
                    {
                        DrawOpeningWorldOverlay(&openingFlow, &plyr, &npcpool);
                        DrawNPCs(&npcpool);
                    }
                    DrawPlayer(&plyr);
                    EndMode2D();

                    // Draw bold mission text at top center
                    const char* missionText = "MISSION: FIND RONDY";
                    if (openingFlow.phase == OPENING_COMPLETE || tutorialFlow.isActive)
                    {
                        missionText = "MISSION: TUTORIAL";
                    }
                    int missionWidth = MeasureText(missionText, 48);
                    int screenCenterX = GetScreenWidth() / 2;
                    int missionX = screenCenterX - missionWidth / 2;
                    DrawText(missionText, missionX + 2, 22, 48, (Color){20, 20, 20, 220});
                    DrawText(missionText, missionX, 20, 48, (Color){255, 210, 60, 255});

                    // Player coordinate readout at top-left.
                    DrawText(TextFormat("X: %.1f  Y: %.1f", plyr.pos.x, plyr.pos.y), 24, 24, 30, (Color){235, 240, 245, 255});

                    // Restored player combat HUD on the left side (legacy style).
                    {
                        WeaponInfo winfo = GetWeaponInfo(&plyr.weapon);
                        DrawText(TextFormat("Health: %d", plyr.health), 10, 60, 30, RED);
                        DrawText(TextFormat("Shield: %d", plyr.shield), 10, 100, 30, BLUE);
                        DrawText(TextFormat("Magazine: %d/%d", winfo.magazine, plyr.weapon.maxMagazine), 10, 150, 30, YELLOW);
                        DrawText(TextFormat("Total Ammo: %d", winfo.totalAmmo), 10, 190, 30, YELLOW);
                        DrawReload(&winfo);
                    }

                    // Minimap at top-right.
                    {
                        const float mapW = 320.0f;
                        const float mapH = 220.0f;
                        const float margin = 26.0f;
                        Rectangle mini = {
                            (float)GetScreenWidth() - mapW - margin,
                            margin,
                            mapW,
                            mapH
                        };

                        if (room.bounds.width > 0.0f && room.bounds.height > 0.0f)
                        {
                            float innerPad = 12.0f;
                            Rectangle mapArea = {
                                mini.x + innerPad,
                                mini.y + innerPad,
                                mini.width - innerPad * 2.0f,
                                mini.height - innerPad * 2.0f
                            };

                            // Raw minimap without title/frame.
                            DrawRectangleRec(mapArea, (Color){35, 48, 68, 210});

                            // Subtle grid to resemble map layout.
                            for (int gx = 1; gx < 8; gx++)
                            {
                                float x = mapArea.x + (mapArea.width / 8.0f) * (float)gx;
                                DrawLine((int)x, (int)mapArea.y, (int)x, (int)(mapArea.y + mapArea.height), (Color){70, 90, 120, 90});
                            }
                            for (int gy = 1; gy < 6; gy++)
                            {
                                float y = mapArea.y + (mapArea.height / 6.0f) * (float)gy;
                                DrawLine((int)mapArea.x, (int)y, (int)(mapArea.x + mapArea.width), (int)y, (Color){70, 90, 120, 90});
                            }

                            float nx = (plyr.pos.x - room.bounds.x) / room.bounds.width;
                            float ny = (plyr.pos.y - room.bounds.y) / room.bounds.height;
                            if (nx < 0.0f) nx = 0.0f;
                            if (nx > 1.0f) nx = 1.0f;
                            if (ny < 0.0f) ny = 0.0f;
                            if (ny > 1.0f) ny = 1.0f;

                            Vector2 p = {
                                mapArea.x + nx * mapArea.width,
                                mapArea.y + ny * mapArea.height
                            };

                            // Door marker.
                            if (!tutorialFlow.isActive && openingFlow.door.width > 0.0f && openingFlow.door.height > 0.0f)
                            {
                                float doorNx = (openingFlow.door.x + openingFlow.door.width * 0.5f - room.bounds.x) / room.bounds.width;
                                float doorNy = (openingFlow.door.y + openingFlow.door.height * 0.5f - room.bounds.y) / room.bounds.height;
                                if (doorNx >= 0.0f && doorNx <= 1.0f && doorNy >= 0.0f && doorNy <= 1.0f)
                                {
                                    Vector2 d = {
                                        mapArea.x + doorNx * mapArea.width,
                                        mapArea.y + doorNy * mapArea.height
                                    };
                                    DrawRectangle((int)d.x - 4, (int)d.y - 4, 8, 8, (Color){245, 210, 95, 255});
                                }
                            }

                            // NPC markers.
                            if (!tutorialFlow.isActive)
                            {
                                for (int i = 0; i < npcpool.count; i++)
                                {
                                    if (!npcpool.npcs[i].active) continue;
                                    float nNx = (npcpool.npcs[i].pos.x - room.bounds.x) / room.bounds.width;
                                    float nNy = (npcpool.npcs[i].pos.y - room.bounds.y) / room.bounds.height;
                                    if (nNx < 0.0f || nNx > 1.0f || nNy < 0.0f || nNy > 1.0f) continue;
                                    Vector2 n = {
                                        mapArea.x + nNx * mapArea.width,
                                        mapArea.y + nNy * mapArea.height
                                    };
                                    DrawCircleV(n, 3.0f, (Color){95, 235, 170, 255});
                                }
                            }

                            DrawCircleV(p, 5.0f, (Color){255, 96, 96, 255});
                            DrawCircleLines((int)p.x, (int)p.y, 8.0f, (Color){255, 235, 165, 255});
                        }
                    }

                    if (tutorialFlow.isActive)
                    {
                        DrawTutorialHUD(&tutorialFlow, &plyr);
                    }
                    else
                    {
                        DrawOpeningHUD(&openingFlow, &plyr);
                    }

                    // Draw movement hint with fade-out (positioned near player center)
                    if (!tutorialFlow.isActive && openingFlow.movementHintTimer > 0.0f && !openingFlow.playerHasMovedInOpening) {
                        float hintAlpha = (openingFlow.movementHintTimer / 5.0f);  // Fade from 1.0 to 0.0 over 5 seconds
                        if (hintAlpha > 1.0f) hintAlpha = 1.0f;
                        Color hintColor = (Color){255, 255, 100, (unsigned char)(255 * hintAlpha)};
                        
                        const char* hintText = "Use WASD to move";
                        int hintWidth = MeasureText(hintText, 36);
                        int screenCenterX = GetScreenWidth() / 2;
                        int screenCenterY = GetScreenHeight() / 2 - 150;  // Above player, but not too high
                        
                        DrawText(hintText, screenCenterX - hintWidth / 2, screenCenterY, 36, hintColor);
                    }

                    break;
                }

                case STATE_SCENE:
                    // Draw the scene (dialog system)
                    DrawScene(&currentScene);
                    break;
            }
        EndDrawing();
    }
    if (titleBackgroundLoaded) UnloadTexture(titleBackground);
    ShutdownMapSystem();
    CloseWindow();
    return 0;
}