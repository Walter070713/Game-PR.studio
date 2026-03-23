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
#include "CombatRuntime.h"
#include "Progression.h"
#include "TutorialPhase.h"

// Main architecture:
// - STATE_TITLE / STATE_SETTINGS / STATE_SCENE are UI-driven states.
// - STATE_GAMEPLAY is chapter-driven (Opening -> Tutorial -> Level).
// - Opening/Tutorial are peaceful objective flows; Level enables combat runtime.

// All the code done so far is coded by Walter from 6th Mar to 14th Mar
// Mostly from 18:30 to 24:00, sometimes to 3:00 am
// Weapon system on 17th Mar


// Basic window settings
int window_width = 2560;
int window_height = 1600;
bool isFullscreen = false;
Vector2 window_center;

// Maximum possible bullet slots used by any weapon
#define MAX_BULLET_POOL 60

static void StartOpeningMission(OpeningFlow* openingFlow, Scene* currentScene, GameState* currentScreen)
{
    // Enter story scene mode immediately after mission start.
    OpeningStartMission(openingFlow, currentScene);
    *currentScreen = STATE_SCENE;
}

static void StartLevelOne(GameProgression* progression, GameMap* room, Player* player,
    Enemy enemyPool[], int enemyCapacity, Bullet bulletPool[], int bulletCapacity, float* spawnTimer)
{
    if (!progression || !room || !player || !enemyPool || !bulletPool || !spawnTimer) return;

    // Prepare first combat-ready chapter and reset runtime pools.
    ProgressionSetLevel(progression, 1, true);
    *room = InitRoom();

    player->pos = (Vector2){room->bounds.x + 240.0f, room->bounds.y + room->bounds.height * 0.5f};
    player->prevpos = player->pos;

    ResetCombatRuntime(enemyPool, enemyCapacity, bulletPool, bulletCapacity, spawnTimer);
}

int main(void) {
    const int emy_capacity = 5; // max enemy capacity

    // Initialize window center
    window_center = (Vector2){(float)window_width/2, (float)window_height/2};

    // State Initialization
    GameState currentScreen = STATE_TITLE;

    // Scene system
    Scene currentScene = {0};
    OpeningFlow openingFlow = {0};
    TutorialFlow tutorialFlow = {0};
    GameProgression progression = {0};

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
    InitOpeningFlow(&openingFlow);
    InitTutorialFlow(&tutorialFlow);
    InitProgression(&progression);
    InitWindow(window_width, window_height, "GAME by PR.studio");


    while (!WindowShouldClose())
    {
        // Global input handling
        if (IsKeyPressed(KEY_F11))  ToggleFullscreenMode();
        
        switch (currentScreen) 
        {
            case STATE_TITLE:
                if (IsKeyPressed(KEY_ENTER)) {
                    StartOpeningMission(&openingFlow, &currentScene, &currentScreen);
                    ProgressionSetOpening(&progression);
                    InitTutorialFlow(&tutorialFlow);
                }
                break;

            case STATE_SETTINGS:
                if (IsKeyPressed(KEY_ESCAPE)) currentScreen = STATE_TITLE;
                break;
            case STATE_GAMEPLAY: {
                // Gameplay update pass: chapter routes own logic.
                bool shouldEnterScene = false;
                UpdatePlayerPos(&plyr); // Player movement logic

                switch (progression.chapter)
                {
                    case CHAPTER_OPENING:
                        // Opening flow may request a temporary scene transition.
                        if (UpdateOpeningPeacefulPhase(&openingFlow, &plyr, &room, enemypool, emy_capacity, bulletpool, &SpawnTimer, &currentScene, &shouldEnterScene)) {
                            if (shouldEnterScene) {
                                currentScreen = STATE_SCENE;
                            }
                        }

                        // Once opening is complete, move into tutorial chapter.
                        if (OpeningIsComplete(&openingFlow)) {
                            ProgressionSetTutorial(&progression);
                            StartTutorialFlow(
                                &tutorialFlow,
                                &room,
                                &plyr,
                                bulletpool,
                                GetWeaponBulletPoolSize(&plyr.weapon),
                                enemypool,
                                emy_capacity,
                                &SpawnTimer
                            );
                        }
                        break;

                    case CHAPTER_TUTORIAL:
                        // Tutorial flow is also peaceful and objective-based.
                        UpdateTutorialFlow(&tutorialFlow, &plyr, &room, enemypool, bulletpool);

                        // Promote to Level 1 after tutorial objective is done.
                        if (tutorialFlow.isComplete) {
                            StartLevelOne(
                                &progression,
                                &room,
                                &plyr,
                                enemypool,
                                emy_capacity,
                                bulletpool,
                                GetWeaponBulletPoolSize(&plyr.weapon),
                                &SpawnTimer
                            );
                        }
                        break;

                    case CHAPTER_LEVEL:
                        // Combat runtime is isolated to level chapters.
                        if (progression.combatEnabled) {
                            UpdateCombatRuntime(
                                &plyr,
                                enemypool,
                                emy_capacity,
                                bulletpool,
                                GetWeaponBulletPoolSize(&plyr.weapon),
                                &mouse,
                                camera,
                                room,
                                &SpawnTimer,
                                SpawnRate
                            );
                        } else {
                            // Fallback for non-combat levels: movement and map collision only.
                            ResolveMapCollisions(&plyr, room, enemypool, 0, bulletpool, 0);
                        }
                        break;
                }

                camera.target = Vector2Lerp(plyr.pos, camera.target, 0.001f); // To keep the player is always at center of the screen
                
                break;
            }

            case STATE_SCENE:
                // Update scene state
                if (!UpdateScene(&currentScene)) {
                    // Scene completion hook per chapter; opening currently owns scene callbacks.
                    if (progression.chapter == CHAPTER_OPENING) {
                        OpeningHandleSceneComplete(
                            &openingFlow,
                            &room,
                            &plyr,
                            bulletpool,
                            GetWeaponBulletPoolSize(&plyr.weapon),
                            enemypool,
                            emy_capacity,
                            &SpawnTimer
                        );
                    }

                    // Scene finished, return to gameplay
                    currentScreen = STATE_GAMEPLAY;
                }
                break;
        }

        // Render pass follows current game state.
        BeginDrawing();
            ClearBackground(BLACK);
            switch (currentScreen) 
            {
                case STATE_TITLE:
                    DrawText("GAME-PR.STUDIO", 100, 100, 80, RED);
                    
                    if (IsOptionClicked("START MISSION", 100, 300, 40, WHITE, YELLOW)) 
                    {
                        StartOpeningMission(&openingFlow, &currentScene, &currentScreen);
                        ProgressionSetOpening(&progression);
                        InitTutorialFlow(&tutorialFlow);
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
                    DrawText("- F11 to Toggle Fullscreen", 100, 320, 30, GRAY);
                    
                    // Resolution options
                    DrawText("RESOLUTION:", 100, 380, 40, WHITE);
                    if (IsOptionClicked("2560 x 1600", 120, 430, 30, WHITE, YELLOW)) 
                    {
                        if (isFullscreen) ToggleFullscreenMode(); // Exit fullscreen first
                        ChangeResolution(2560, 1600, &camera);
                    }
                    if (IsOptionClicked("3200 x 2000", 120, 470, 30, WHITE, YELLOW)) 
                    {
                        if (isFullscreen) ToggleFullscreenMode(); // Exit fullscreen first
                        ChangeResolution(3200, 2000, &camera);
                    }
                    if (IsOptionClicked("1920 x 1080", 120, 510, 30, WHITE, YELLOW)) 
                    {
                        if (isFullscreen) ToggleFullscreenMode(); // Exit fullscreen first
                        ChangeResolution(1920, 1080, &camera);
                    }
                    
                    // Fullscreen toggle
                    DrawText("DISPLAY:", 100, 570, 40, WHITE);
                    char* fullscreenText = isFullscreen ? "WINDOWED MODE" : "FULLSCREEN MODE";
                    if (IsOptionClicked(fullscreenText, 120, 620, 30, WHITE, YELLOW))  ToggleFullscreenMode();
                    
                    if (IsOptionClicked("BACK TO MENU", 100, 700, 40, WHITE, GREEN))  currentScreen = STATE_TITLE;

                    break;
                case STATE_GAMEPLAY:
                {
                    // Gameplay render pass mirrors chapter ownership.
                    bool combatEnabled = progression.chapter == CHAPTER_LEVEL && progression.combatEnabled;

                    BeginMode2D(camera);
                    DrawMap(room); // room

                    if (progression.chapter == CHAPTER_OPENING) {
                        // Opening world cues (door + console block).
                        DrawOpeningWorldOverlay(&openingFlow);
                    } else if (progression.chapter == CHAPTER_TUTORIAL) {
                        // Tutorial world cue (terminal).
                        DrawTutorialWorldOverlay(&tutorialFlow);
                    }

                    if (combatEnabled) {
                        DrawCombatRuntimeWorld(
                            &plyr,
                            enemypool,
                            emy_capacity,
                            bulletpool,
                            GetWeaponBulletPoolSize(&plyr.weapon),
                            &mouse
                        );
                    }
                    DrawPlayer(&plyr); // Player
                    EndMode2D();
                    
                    DrawText(plyr.name, 10, 10, 40, YELLOW);

                    if (combatEnabled) {
                        DrawCombatRuntimeHUD(&plyr);
                    }

                    if (progression.chapter == CHAPTER_OPENING) {
                        DrawOpeningHUD(&openingFlow, &plyr);
                    } else if (progression.chapter == CHAPTER_TUTORIAL) {
                        DrawTutorialHUD(&tutorialFlow, &plyr);
                    } else {
                        // Basic level label until dedicated level HUD is introduced.
                        DrawText(TextFormat("Level %d", progression.levelIndex), 10, 230, 30, ORANGE);
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
    CloseWindow();
    return 0;
}