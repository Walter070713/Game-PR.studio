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


// Basic window settings
int window_width = 2560;
int window_height = 1600;
bool isFullscreen = false;
Vector2 window_center;

// Maximum possible bullet slots used by any weapon
#define MAX_BULLET_POOL 60

static void DrawMiniMap(GameMap room, const Player* player, GameProgression progression,
    const OpeningFlow* openingFlow, const TutorialFlow* tutorialFlow)
{
    if (!player || !openingFlow || !tutorialFlow) return;

    const float panelWidth = 260.0f;
    const float panelHeight = 190.0f;
    const float margin = 20.0f;
    const float innerPad = 10.0f;

    float panelX = (float)GetScreenWidth() - panelWidth - margin;
    float panelY = margin;

    float mapViewX = panelX + innerPad;
    float mapViewY = panelY + innerPad;
    float mapViewW = panelWidth - innerPad * 2.0f;
    float mapViewH = panelHeight - innerPad * 2.0f;

    float sx = mapViewW / room.bounds.width;
    float sy = mapViewH / room.bounds.height;
    float scale = (sx < sy) ? sx : sy;

    float drawW = room.bounds.width * scale;
    float drawH = room.bounds.height * scale;
    float drawX = mapViewX + (mapViewW - drawW) * 0.5f;
    float drawY = mapViewY + (mapViewH - drawH) * 0.5f;

    DrawRectangle((int)panelX, (int)panelY, (int)panelWidth, (int)panelHeight, (Color){18, 22, 28, 140});
    DrawRectangle((int)drawX, (int)drawY, (int)drawW, (int)drawH, (Color){56, 64, 76, 255});

    for (int i = 0; i < room.WallCount; i++)
    {
        Rectangle w = room.walls[i];
        float wx = drawX + (w.x - room.bounds.x) * scale;
        float wy = drawY + (w.y - room.bounds.y) * scale;
        float ww = w.width * scale;
        float wh = w.height * scale;
        DrawRectangle((int)wx, (int)wy, (int)ww, (int)wh, GRAY);
    }

    if (progression.chapter == CHAPTER_OPENING && openingFlow->phase == OPENING_SMALL_ROOM)
    {
        float bx = drawX + (openingFlow->interactBlock.x - room.bounds.x) * scale;
        float by = drawY + (openingFlow->interactBlock.y - room.bounds.y) * scale;
        float bw = openingFlow->interactBlock.width * scale;
        float bh = openingFlow->interactBlock.height * scale;
        DrawRectangle((int)bx, (int)by, (int)bw, (int)bh, SKYBLUE);

        float dx = drawX + (openingFlow->door.x - room.bounds.x) * scale;
        float dy = drawY + (openingFlow->door.y - room.bounds.y) * scale;
        float dw = openingFlow->door.width * scale;
        float dh = openingFlow->door.height * scale;
        DrawRectangle((int)dx, (int)dy, (int)dw, (int)dh, BROWN);
    }

    if (progression.chapter == CHAPTER_TUTORIAL && tutorialFlow->isActive)
    {
        float tx = drawX + (tutorialFlow->terminal.x - room.bounds.x) * scale;
        float ty = drawY + (tutorialFlow->terminal.y - room.bounds.y) * scale;
        float tw = tutorialFlow->terminal.width * scale;
        float th = tutorialFlow->terminal.height * scale;
        DrawRectangle((int)tx, (int)ty, (int)tw, (int)th, VIOLET);
    }

    float px = drawX + (player->pos.x - room.bounds.x) * scale;
    float py = drawY + (player->pos.y - room.bounds.y) * scale;
    if (px < drawX) px = drawX;
    if (px > drawX + drawW) px = drawX + drawW;
    if (py < drawY) py = drawY;
    if (py > drawY + drawH) py = drawY + drawH;
    DrawCircle((int)px, (int)py, 5.0f, YELLOW);
}

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

                // Soul Knight-like camera: player remains centered, void can be visible outside room.
                camera.target = plyr.pos;
                
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

                    DrawMiniMap(room, &plyr, progression, &openingFlow, &tutorialFlow);

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