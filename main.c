#include "raylib.h"
#include "raymath.h"
#include "window_setting.h"
#include "Player.h"
#include "CameraSet.h"
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
#include "Progression.h"
#include <stdio.h>
#include <string.h>

// Main architecture:
// - STATE_TITLE / STATE_SETTINGS / STATE_SCENE are UI-driven states.
// - STATE_GAMEPLAY runs TMX map exploration with door interaction.

// Maximum possible bullet slots used by any weapon
#define MAX_BULLET_POOL 60

// Save-file format metadata.
#define SAVE_FILE_PATH "savegame.dat"
#define SAVE_MAGIC 0x45584543u
#define SAVE_VERSION 1
#define SAVE_MAX_NAME_LEN 24
#define SAVE_MAX_TEXT_LEN 32
#define SAVE_MAX_ENEMY_SLOTS 5
#define SAVE_MAX_NPC_SLOTS 16
#define SAVE_MAX_DOOR_ZONES 16

// Serializable enemy snapshot entry.
typedef struct {
    Vector2 pos;
    Vector2 prevpos;
    Color state;
    float body;
    float speed;
    int health;
    bool active;
    float flashtime;
    char name[SAVE_MAX_TEXT_LEN];
} SavedEnemy;

// Serializable NPC snapshot entry.
typedef struct {
    Vector2 pos;
    float body;
    Color color;
    bool active;
    char name[SAVE_MAX_TEXT_LEN];
} SavedNPC;

// Full save snapshot for restoring a gameplay session.
typedef struct {
    unsigned int magic;
    int version;
    char playerName[SAVE_MAX_NAME_LEN];

    Vector2 playerPos;
    Vector2 playerPrevPos;
    Vector2 playerDir;
    float playerSpeed;
    float playerBody;
    int playerHealth;
    int playerShield;
    int playerMaxShield;
    float playerHurtTimer;
    float playerShieldRegenTimer;
    float playerShieldRegenAccum;
    Weapon playerWeapon;

    OpeningFlow openingFlow;
    TutorialFlow tutorialFlow;
    GameProgression progression;
    float spawnTimer;

    int enemyCount;
    SavedEnemy enemies[SAVE_MAX_ENEMY_SLOTS];

    int bulletCount;
    Bullet bullets[MAX_BULLET_POOL];

    int npcCount;
    SavedNPC npcs[SAVE_MAX_NPC_SLOTS];

    char activeMapPath[260];
    int openedDoorCount;
    Rectangle openedDoors[SAVE_MAX_DOOR_ZONES];
} SaveGameData;

// Stable backing storage for loaded name pointers.
static char gLoadedEnemyNames[SAVE_MAX_ENEMY_SLOTS][SAVE_MAX_TEXT_LEN] = {{0}};
static char gLoadedNPCNames[SAVE_MAX_NPC_SLOTS][SAVE_MAX_TEXT_LEN] = {{0}};

// Start opening chapter scene and transition to scene state.
static void StartOpeningMission(OpeningFlow* openingFlow, Scene* currentScene, GameState* currentScreen,
    GameProgression* progression, const Player* player)
{
    ProgressionSetOpening(progression);

    // Start opening scene first, then map gameplay after scene completion.
    OpeningStartMission(openingFlow, currentScene, player);
    *currentScreen = STATE_SCENE;
}

// Collect keyboard character input into fixed-size player name buffer.
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

// Update camera zoom/offset and tile-relative player movement speed.
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

// Safe bounded string copy helper used by save/load routines.
static void CopyStringSafe(char* dst, int dstSize, const char* src)
{
    if (!dst || dstSize <= 0) return;
    if (!src)
    {
        dst[0] = '\0';
        return;
    }

    strncpy(dst, src, (size_t)(dstSize - 1));
    dst[dstSize - 1] = '\0';
}

// Quick validation check for existence + signature/version of save file.
static bool HasValidSaveGame(void)
{
    FILE* file;
    SaveGameData save = {0};

    file = fopen(SAVE_FILE_PATH, "rb");
    if (!file) return false;

    if (fread(&save, sizeof(save), 1, file) != 1)
    {
        fclose(file);
        return false;
    }

    fclose(file);
    return save.magic == SAVE_MAGIC && save.version == SAVE_VERSION;
}

// Delete save file (used by reset/new game flows).
static void DeleteSaveGame(void)
{
    remove(SAVE_FILE_PATH);
}

// Serialize current runtime state to binary save snapshot.
static bool SaveGameSnapshot(const char* playerName, const Player* player, const OpeningFlow* openingFlow,
    const TutorialFlow* tutorialFlow, const GameProgression* progression, float spawnTimer,
    const Enemy enemyPool[], int enemyCapacity, const Bullet bulletPool[], int bulletCapacity,
    const NPCPool* npcPool)
{
    FILE* file;
    SaveGameData save = {0};

    if (!playerName || !player || !openingFlow || !tutorialFlow || !progression || !enemyPool || !bulletPool || !npcPool)
    {
        return false;
    }

    save.magic = SAVE_MAGIC;
    save.version = SAVE_VERSION;

    CopyStringSafe(save.playerName, (int)sizeof(save.playerName), playerName);
    save.playerPos = player->pos;
    save.playerPrevPos = player->prevpos;
    save.playerDir = player->dir;
    save.playerSpeed = player->speed;
    save.playerBody = player->body;
    save.playerHealth = player->health;
    save.playerShield = player->shield;
    save.playerMaxShield = player->maxshield;
    save.playerHurtTimer = player->hurtTimer;
    save.playerShieldRegenTimer = player->shieldRegenTimer;
    save.playerShieldRegenAccum = player->shieldRegenAccum;
    save.playerWeapon = player->weapon;

    save.openingFlow = *openingFlow;
    save.tutorialFlow = *tutorialFlow;
    save.progression = *progression;
    save.spawnTimer = spawnTimer;

    save.enemyCount = enemyCapacity;
    if (save.enemyCount > SAVE_MAX_ENEMY_SLOTS) save.enemyCount = SAVE_MAX_ENEMY_SLOTS;
    if (save.enemyCount < 0) save.enemyCount = 0;

    for (int i = 0; i < save.enemyCount; i++)
    {
        save.enemies[i].pos = enemyPool[i].pos;
        save.enemies[i].prevpos = enemyPool[i].prevpos;
        save.enemies[i].state = enemyPool[i].state;
        save.enemies[i].body = enemyPool[i].body;
        save.enemies[i].speed = enemyPool[i].speed;
        save.enemies[i].health = enemyPool[i].health;
        save.enemies[i].active = enemyPool[i].active;
        save.enemies[i].flashtime = enemyPool[i].flashtime;
        CopyStringSafe(save.enemies[i].name, (int)sizeof(save.enemies[i].name), enemyPool[i].name);
    }

    save.bulletCount = bulletCapacity;
    if (save.bulletCount > MAX_BULLET_POOL) save.bulletCount = MAX_BULLET_POOL;
    if (save.bulletCount < 0) save.bulletCount = 0;
    for (int i = 0; i < save.bulletCount; i++) save.bullets[i] = bulletPool[i];

    save.npcCount = npcPool->count;
    if (save.npcCount > SAVE_MAX_NPC_SLOTS) save.npcCount = SAVE_MAX_NPC_SLOTS;
    if (save.npcCount < 0) save.npcCount = 0;
    for (int i = 0; i < save.npcCount; i++)
    {
        save.npcs[i].pos = npcPool->npcs[i].pos;
        save.npcs[i].body = npcPool->npcs[i].body;
        save.npcs[i].color = npcPool->npcs[i].color;
        save.npcs[i].active = npcPool->npcs[i].active;
        CopyStringSafe(save.npcs[i].name, (int)sizeof(save.npcs[i].name), npcPool->npcs[i].name);
    }

    CopyStringSafe(save.activeMapPath, (int)sizeof(save.activeMapPath), GetActiveTMXMapPath());
    save.openedDoorCount = GetOpenedDoorZones(save.openedDoors, SAVE_MAX_DOOR_ZONES);

    file = fopen(SAVE_FILE_PATH, "wb");
    if (!file) return false;

    if (fwrite(&save, sizeof(save), 1, file) != 1)
    {
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

// Restore runtime state from save snapshot file.
static bool LoadGameSnapshot(char playerName[], int playerNameSize, Player* player, OpeningFlow* openingFlow,
    TutorialFlow* tutorialFlow, GameProgression* progression, float* spawnTimer,
    Enemy enemyPool[], int enemyCapacity, Bullet bulletPool[], int bulletCapacity,
    NPCPool* npcPool, GameMap* room)
{
    FILE* file;
    SaveGameData save = {0};

    if (!playerName || playerNameSize <= 0 || !player || !openingFlow || !tutorialFlow || !progression || !spawnTimer ||
        !enemyPool || !bulletPool || !npcPool || !room)
    {
        return false;
    }

    file = fopen(SAVE_FILE_PATH, "rb");
    if (!file) return false;

    if (fread(&save, sizeof(save), 1, file) != 1)
    {
        fclose(file);
        return false;
    }
    fclose(file);

    if (save.magic != SAVE_MAGIC || save.version != SAVE_VERSION) return false;

    if (save.activeMapPath[0] != '\0')
    {
        if (!SetActiveTMXMap(save.activeMapPath)) return false;
    }
    else
    {
        if (!SetActiveTMXMap("TEST MAP.tmx")) return false;
    }

    *room = InitRoom();
    ResetOpenedDoorZones();
    SetOpenedDoorZones(save.openedDoors, save.openedDoorCount);

    CopyStringSafe(playerName, playerNameSize, (save.playerName[0] != '\0') ? save.playerName : "Player");

    player->pos = save.playerPos;
    player->prevpos = save.playerPrevPos;
    player->dir = save.playerDir;
    player->name = playerName;
    player->speed = save.playerSpeed;
    player->body = save.playerBody;
    player->health = save.playerHealth;
    player->shield = save.playerShield;
    player->maxshield = save.playerMaxShield;
    player->hurtTimer = save.playerHurtTimer;
    player->shieldRegenTimer = save.playerShieldRegenTimer;
    player->shieldRegenAccum = save.playerShieldRegenAccum;
    player->weapon = save.playerWeapon;

    *openingFlow = save.openingFlow;
    *tutorialFlow = save.tutorialFlow;
    *progression = save.progression;
    *spawnTimer = save.spawnTimer;

    for (int i = 0; i < enemyCapacity; i++)
    {
        enemyPool[i].pos = (Vector2){0};
        enemyPool[i].prevpos = (Vector2){0};
        enemyPool[i].state = WHITE;
        enemyPool[i].body = 0.0f;
        enemyPool[i].speed = 0.0f;
        enemyPool[i].health = 0;
        enemyPool[i].active = false;
        enemyPool[i].flashtime = 0.0f;
        enemyPool[i].name = NULL;
    }

    {
        int restoreEnemyCount = save.enemyCount;
        if (restoreEnemyCount > enemyCapacity) restoreEnemyCount = enemyCapacity;
        if (restoreEnemyCount > SAVE_MAX_ENEMY_SLOTS) restoreEnemyCount = SAVE_MAX_ENEMY_SLOTS;
        if (restoreEnemyCount < 0) restoreEnemyCount = 0;

        for (int i = 0; i < restoreEnemyCount; i++)
        {
            enemyPool[i].pos = save.enemies[i].pos;
            enemyPool[i].prevpos = save.enemies[i].prevpos;
            enemyPool[i].state = save.enemies[i].state;
            enemyPool[i].body = save.enemies[i].body;
            enemyPool[i].speed = save.enemies[i].speed;
            enemyPool[i].health = save.enemies[i].health;
            enemyPool[i].active = save.enemies[i].active;
            enemyPool[i].flashtime = save.enemies[i].flashtime;

            CopyStringSafe(gLoadedEnemyNames[i], SAVE_MAX_TEXT_LEN, save.enemies[i].name);
            enemyPool[i].name = (gLoadedEnemyNames[i][0] != '\0') ? gLoadedEnemyNames[i] : NULL;
        }
    }

    InitBulletPool(bulletPool, bulletCapacity);
    {
        int restoreBulletCount = save.bulletCount;
        if (restoreBulletCount > bulletCapacity) restoreBulletCount = bulletCapacity;
        if (restoreBulletCount > MAX_BULLET_POOL) restoreBulletCount = MAX_BULLET_POOL;
        if (restoreBulletCount < 0) restoreBulletCount = 0;

        for (int i = 0; i < restoreBulletCount; i++) bulletPool[i] = save.bullets[i];
    }

    InitNPCPool(npcPool);
    {
        int restoreNpcCount = save.npcCount;
        if (restoreNpcCount > SAVE_MAX_NPC_SLOTS) restoreNpcCount = SAVE_MAX_NPC_SLOTS;
        if (restoreNpcCount < 0) restoreNpcCount = 0;

        npcPool->count = restoreNpcCount;
        for (int i = 0; i < restoreNpcCount; i++)
        {
            npcPool->npcs[i].pos = save.npcs[i].pos;
            npcPool->npcs[i].body = save.npcs[i].body;
            npcPool->npcs[i].color = save.npcs[i].color;
            npcPool->npcs[i].active = save.npcs[i].active;

            CopyStringSafe(gLoadedNPCNames[i], SAVE_MAX_TEXT_LEN, save.npcs[i].name);
            npcPool->npcs[i].name = (gLoadedNPCNames[i][0] != '\0') ? gLoadedNPCNames[i] : NULL;
        }
    }

    return true;
}

// Reset all chapter/runtime systems for starting a clean profile.
static void ResetRuntimeForNewProfile(GameMap* room, Player* player, Enemy enemyPool[], int enemyCapacity,
    NPCPool* npcPool, float* spawnTimer, Bullet bulletPool[], Camera2D* camera,
    OpeningFlow* openingFlow, TutorialFlow* tutorialFlow, GameProgression* progression, Scene* scene)
{
    if (!room || !player || !enemyPool || !npcPool || !spawnTimer || !bulletPool || !camera ||
        !openingFlow || !tutorialFlow || !progression || !scene)
    {
        return;
    }

    SetActiveTMXMap("TEST MAP.tmx");
    *room = InitRoom();
    ResetOpenedDoorZones();

    InitPlayer(player, window_center);
    InitEnemy(enemyPool, enemyCapacity);
    InitNPCPool(npcPool);
    InitBulletPool(bulletPool, MAX_BULLET_POOL);
    InitOpeningFlow(openingFlow);
    InitTutorialFlow(tutorialFlow);
    InitProgression(progression);
    *spawnTimer = 0.0f;

    CleanupScene(scene);
    InitCamera(camera, window_center);
    ApplyDynamicCameraAndMovement(player, room, camera);

    player->pos = (Vector2){room->bounds.x + room->bounds.width * 0.5f, room->bounds.y + room->bounds.height * 0.5f};
    player->prevpos = player->pos;
}

int main(void) {
    const int emy_capacity = 5; // max enemy capacity

    InitWindowSettings(2560, 1600);

    // State Initialization
    // Start from title; progression flows: Title -> Opening Scene -> Opening Map -> Tutorial
    GameState currentScreen = STATE_TITLE;

    // Scene system
    Scene currentScene = {0};
    OpeningFlow openingFlow = {0};
    TutorialFlow tutorialFlow = {0};
    GameProgression progression = {0};
    char playerName[24] = "";
    char pauseMessage[64] = "";
    float pauseMessageTimer = 0.0f;
    bool gameplayPaused = false;
    bool pauseSettingsPanel = false;
    bool hasSaveGame = false;
    bool showNewGameConfirm = false;
    bool showExitConfirm = false;
    bool showCreditsPanel = false;
    bool requestExit = false;
    bool prevWindowClose = false;
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
    SetExitKey(KEY_NULL);

    // Initializations
    GameMap room = InitRoom(); 
    InitPlayer(&plyr, window_center); 
    InitEnemy(enemypool, emy_capacity);
    InitNPCPool(&npcpool);
    InitCamera(&camera, window_center);
    InitBulletPool(bulletpool, MAX_BULLET_POOL);
    InitOpeningFlow(&openingFlow);
    InitTutorialFlow(&tutorialFlow);
    InitProgression(&progression);
    hasSaveGame = HasValidSaveGame();
    ApplyDynamicCameraAndMovement(&plyr, &room, &camera);

    if (FileExists("Assets/backgrounds/2-MC IN SERVER ROOM.png"))
    {
        titleBackground = LoadTexture("Assets/backgrounds/2-MC IN SERVER ROOM.png");
        titleBackgroundLoaded = (titleBackground.id != 0);
    }

    // Spawn player inside the active map bounds for TMX visual testing.
    plyr.pos = (Vector2){room.bounds.x + room.bounds.width * 0.5f, room.bounds.y + room.bounds.height * 0.5f};
    plyr.prevpos = plyr.pos;


    // Main application loop: input -> update -> render.
    while (!requestExit)
    {
        bool escPressed = IsKeyPressed(KEY_ESCAPE);
        bool windowCloseNow = WindowShouldClose();
        bool windowClosePressed = windowCloseNow && !prevWindowClose;
        prevWindowClose = windowCloseNow;

        // Global input handling shared by all states.
        if (pauseMessageTimer > 0.0f)
        {
            pauseMessageTimer -= GetFrameTime();
            if (pauseMessageTimer < 0.0f) pauseMessageTimer = 0.0f;
        }

        if (IsKeyPressed(KEY_F11)) ToggleFullscreenMode();

        if (escPressed)
        {
            if (currentScreen == STATE_GAMEPLAY)
            {
                if (!gameplayPaused)
                {
                    gameplayPaused = true;
                    pauseSettingsPanel = false;
                }
                else if (pauseSettingsPanel)
                {
                    pauseSettingsPanel = false;
                }
                else
                {
                    gameplayPaused = false;
                }
            }
            else if (currentScreen == STATE_TITLE && showNewGameConfirm)
            {
                showNewGameConfirm = false;
            }
            else
            {
                requestExit = true;
            }
        }

        // Treat OS close button separately from ESC so gameplay ESC cannot force-close.
        if (windowClosePressed && !escPressed)
        {
            requestExit = true;
        }
        
        switch (currentScreen) 
        {
            case STATE_TITLE:
                // Enter key: continue save if available, otherwise new-profile flow.
                if (!showExitConfirm && !showCreditsPanel && !showNewGameConfirm && IsKeyPressed(KEY_ENTER)) {
                    if (hasSaveGame)
                    {
                        if (LoadGameSnapshot(playerName, (int)sizeof(playerName), &plyr, &openingFlow, &tutorialFlow,
                            &progression, &SpawnTimer, enemypool, emy_capacity, bulletpool, MAX_BULLET_POOL, &npcpool, &room))
                        {
                            ApplyDynamicCameraAndMovement(&plyr, &room, &camera);
                            gameplayPaused = false;
                            pauseSettingsPanel = false;
                            pauseMessageTimer = 0.0f;
                            showNewGameConfirm = false;
                            showExitConfirm = false;
                            showCreditsPanel = false;
                            currentScreen = STATE_GAMEPLAY;
                        }
                        else
                        {
                            DeleteSaveGame();
                            hasSaveGame = false;
                            playerName[0] = '\0';
                            gameplayPaused = false;
                            pauseSettingsPanel = false;
                            pauseMessageTimer = 0.0f;
                            showNewGameConfirm = false;
                            showExitConfirm = false;
                            showCreditsPanel = false;
                            ResetRuntimeForNewProfile(&room, &plyr, enemypool, emy_capacity, &npcpool, &SpawnTimer,
                                bulletpool, &camera, &openingFlow, &tutorialFlow, &progression, &currentScene);
                            currentScreen = STATE_NAME_ENTRY;
                        }
                    }
                    else
                    {
                        playerName[0] = '\0';
                        gameplayPaused = false;
                        pauseSettingsPanel = false;
                        pauseMessageTimer = 0.0f;
                        showNewGameConfirm = false;
                        showExitConfirm = false;
                        showCreditsPanel = false;
                        ResetRuntimeForNewProfile(&room, &plyr, enemypool, emy_capacity, &npcpool, &SpawnTimer,
                            bulletpool, &camera, &openingFlow, &tutorialFlow, &progression, &currentScene);
                        currentScreen = STATE_NAME_ENTRY;
                    }
                }
                break;

            case STATE_NAME_ENTRY:
                // Capture display name and start opening chapter.
                CaptureNameInput(playerName, (int)sizeof(playerName));
                if (IsKeyPressed(KEY_ENTER))
                {
                    if (playerName[0] == '\0')
                    {
                        strcpy(playerName, "Player");
                    }
                    plyr.name = playerName;
                    StartOpeningMission(&openingFlow, &currentScene, &currentScreen, &progression, &plyr);
                }
                break;

            case STATE_SETTINGS:
                break;
            case STATE_GAMEPLAY: {
                // Keep camera scaling and movement tuning synced with active map.
                ApplyDynamicCameraAndMovement(&plyr, &room, &camera);

                if (gameplayPaused) break;

                // Keep NPC body size aligned with player body after dynamic scaling.
                for (int i = 0; i < npcpool.count; i++)
                {
                    if (npcpool.npcs[i].active) npcpool.npcs[i].body = plyr.body;
                }

                UpdatePlayerPos(&plyr);

                if (progression.chapter == CHAPTER_TUTORIAL && tutorialFlow.isActive)
                {
                    // Tutorial chapter uses wave-based combat update pipeline.
                    UpdateTutorialFlow(&tutorialFlow, &plyr, &room, enemypool, emy_capacity, bulletpool, GetWeaponBulletPoolSize(&plyr.weapon), camera);
                }
                else
                {
                    // Opening chapter runs exploration + scene transitions.
                    bool shouldEnterScene = false;
                    bool shouldStartTutorial = false;
                    UpdateOpeningPeacefulPhase(
                        &openingFlow,
                        &plyr,
                        &room,
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
                        // Promote progression and initialize tutorial combat room.
                        ProgressionSetTutorial(&progression);
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
                // Update active scene and return to gameplay when complete.
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

            // Render pass follows current state after update step.
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

                    if (!showExitConfirm && !showCreditsPanel && !showNewGameConfirm)
                    {
                        int newGameY = hasSaveGame ? 446 : 360;
                        int settingsY = hasSaveGame ? 532 : 446;
                        int creditsY = hasSaveGame ? 618 : 532;
                        int exitY = hasSaveGame ? 704 : 618;

                        if (hasSaveGame && IsOptionClicked("CONTINUE...", 124, 360, 54, WHITE, GREEN))
                        {
                            if (LoadGameSnapshot(playerName, (int)sizeof(playerName), &plyr, &openingFlow, &tutorialFlow,
                                &progression, &SpawnTimer, enemypool, emy_capacity, bulletpool, MAX_BULLET_POOL, &npcpool, &room))
                            {
                                ApplyDynamicCameraAndMovement(&plyr, &room, &camera);
                                gameplayPaused = false;
                                pauseSettingsPanel = false;
                                pauseMessageTimer = 0.0f;
                                showNewGameConfirm = false;
                                showExitConfirm = false;
                                showCreditsPanel = false;
                                currentScreen = STATE_GAMEPLAY;
                            }
                            else
                            {
                                DeleteSaveGame();
                                hasSaveGame = false;
                            }
                        }

                        if (IsOptionClicked("NEW GAME", 124, newGameY, 54, WHITE, YELLOW))
                        {
                            if (hasSaveGame)
                            {
                                showNewGameConfirm = true;
                            }
                            else
                            {
                                DeleteSaveGame();
                                hasSaveGame = false;
                                playerName[0] = '\0';
                                gameplayPaused = false;
                                pauseSettingsPanel = false;
                                pauseMessageTimer = 0.0f;
                                showNewGameConfirm = false;
                                showExitConfirm = false;
                                showCreditsPanel = false;
                                ResetRuntimeForNewProfile(&room, &plyr, enemypool, emy_capacity, &npcpool, &SpawnTimer,
                                    bulletpool, &camera, &openingFlow, &tutorialFlow, &progression, &currentScene);
                                currentScreen = STATE_NAME_ENTRY;
                            }
                        }
                        if (IsOptionClicked("SETTING", 124, settingsY, 54, WHITE, YELLOW))
                        {
                            currentScreen = STATE_SETTINGS;
                        }
                        if (IsOptionClicked("CREDITS", 124, creditsY, 54, WHITE, YELLOW))
                        {
                            showCreditsPanel = true;
                        }
                        if (IsOptionClicked("EXIT", 124, exitY, 54, WHITE, RED))
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

                    if (showNewGameConfirm)
                    {
                        int sw = GetScreenWidth();
                        int sh = GetScreenHeight();
                        Rectangle modal = {(float)sw * 0.5f - 320.0f, (float)sh * 0.5f - 150.0f, 640.0f, 300.0f};
                        DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 165});
                        DrawRectangleRec(modal, (Color){30, 34, 44, 242});
                        DrawRectangleLinesEx(modal, 3.0f, (Color){240, 202, 90, 255});
                        DrawText("START NEW GAME?", (int)modal.x + 108, (int)modal.y + 34, 50, (Color){250, 240, 205, 255});
                        DrawText("Your existing save will be deleted.", (int)modal.x + 112, (int)modal.y + 112, 28, LIGHTGRAY);

                        if (IsOptionClicked("YES", (int)modal.x + 150, (int)modal.y + 206, 38, WHITE, RED))
                        {
                            DeleteSaveGame();
                            hasSaveGame = false;
                            playerName[0] = '\0';
                            gameplayPaused = false;
                            pauseSettingsPanel = false;
                            pauseMessageTimer = 0.0f;
                            showNewGameConfirm = false;
                            showExitConfirm = false;
                            showCreditsPanel = false;
                            ResetRuntimeForNewProfile(&room, &plyr, enemypool, emy_capacity, &npcpool, &SpawnTimer,
                                bulletpool, &camera, &openingFlow, &tutorialFlow, &progression, &currentScene);
                            currentScreen = STATE_NAME_ENTRY;
                        }
                        if (IsOptionClicked("NO", (int)modal.x + 390, (int)modal.y + 206, 38, WHITE, GREEN))
                        {
                            showNewGameConfirm = false;
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

                    DrawText("ENTER: Confirm  |  BACKSPACE: Delete  |  ESC: Quit", 100, 420, 26, LIGHTGRAY);

                    bool canStart = playerName[0] != '\0';
                    Color startColor = canStart ? GREEN : DARKGRAY;
                    if (IsOptionClicked("BEGIN", 100, 500, 42, startColor, YELLOW))
                    {
                        if (playerName[0] == '\0') strcpy(playerName, "Player");
                        plyr.name = playerName;
                        StartOpeningMission(&openingFlow, &currentScene, &currentScreen, &progression, &plyr);
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
                        ApplyWindowedResolution(2560, 1600, &camera);
                    }
                    if (IsOptionClicked("3200 x 2000", 140, 590, 32, WHITE, YELLOW))
                    {
                        ApplyWindowedResolution(3200, 2000, &camera);
                    }
                    if (IsOptionClicked("1920 x 1080", 140, 635, 32, WHITE, YELLOW))
                    {
                        ApplyWindowedResolution(1920, 1080, &camera);
                    }

                    DrawText("DISPLAY MODE", 124, 686, 34, (Color){200, 226, 255, 255});
                    {
                        char* fullscreenText = isFullscreen ? "WINDOWED MODE" : "FULLSCREEN MODE";
                        if (IsOptionClicked(fullscreenText, 140, 728, 30, WHITE, YELLOW)) ToggleFullscreenMode();
                    }

                    DrawText(TextFormat("SCENE TEXT SPEED: %.0f cps", GetSceneTextSpeed()), 124, 770, 30, (Color){200, 226, 255, 255});
                    if (IsOptionClicked("SLOWER (-)", 140, 806, 28, WHITE, YELLOW))
                    {
                        SetSceneTextSpeed(GetSceneTextSpeed() - 6.0f);
                    }
                    if (IsOptionClicked("FASTER (+)", 320, 806, 28, WHITE, YELLOW))
                    {
                        SetSceneTextSpeed(GetSceneTextSpeed() + 6.0f);
                    }

                    if (IsOptionClicked("BACK TO MENU", 700, 804, 34, WHITE, GREEN)) currentScreen = STATE_TITLE;

                    break;
                }
                case STATE_GAMEPLAY:
                {
                    BeginMode2D(camera);
                    DrawMap(room);
                    if (tutorialFlow.isActive)
                    {
                        DrawTutorialWorldOverlay(&plyr, enemypool, emy_capacity, bulletpool, GetWeaponBulletPoolSize(&plyr.weapon), camera);
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

                    // Player combat HUD on the left side.
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
                        DrawTutorialHUD(&tutorialFlow);
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

                    if (gameplayPaused)
                    {
                        int sw = GetScreenWidth();
                        int sh = GetScreenHeight();
                        Rectangle modal = {(float)sw * 0.5f - 320.0f, (float)sh * 0.5f - 220.0f, 640.0f, 440.0f};

                        DrawRectangle(0, 0, sw, sh, (Color){0, 0, 0, 170});
                        DrawRectangleRec(modal, (Color){30, 34, 44, 242});
                        DrawRectangleLinesEx(modal, 3.0f, (Color){240, 202, 90, 255});

                        if (!pauseSettingsPanel)
                        {
                            DrawText("PAUSED", (int)modal.x + 240, (int)modal.y + 36, 62, (Color){250, 240, 205, 255});

                            if (IsOptionClicked("CONTINUE", (int)modal.x + 220, (int)modal.y + 130, 44, WHITE, GREEN))
                            {
                                gameplayPaused = false;
                            }
                            if (IsOptionClicked("SETTINGS", (int)modal.x + 220, (int)modal.y + 188, 44, WHITE, YELLOW))
                            {
                                pauseSettingsPanel = true;
                            }
                            if (IsOptionClicked("SAVE", (int)modal.x + 220, (int)modal.y + 246, 44, WHITE, ORANGE))
                            {
                                if (SaveGameSnapshot(playerName, &plyr, &openingFlow, &tutorialFlow, &progression,
                                    SpawnTimer, enemypool, emy_capacity, bulletpool,
                                    GetWeaponBulletPoolSize(&plyr.weapon), &npcpool))
                                {
                                    hasSaveGame = true;
                                    gameplayPaused = false;
                                    pauseSettingsPanel = false;
                                    pauseMessageTimer = 0.0f;
                                    showNewGameConfirm = false;
                                    showExitConfirm = false;
                                    showCreditsPanel = false;
                                    currentScreen = STATE_TITLE;
                                }
                                else
                                {
                                    CopyStringSafe(pauseMessage, (int)sizeof(pauseMessage), "Save failed.");
                                    pauseMessageTimer = 2.5f;
                                }
                            }
                            if (IsOptionClicked("EXIT", (int)modal.x + 220, (int)modal.y + 304, 44, WHITE, RED))
                            {
                                requestExit = true;
                            }
                        }
                        else
                        {
                            DrawText("PAUSE SETTINGS", (int)modal.x + 150, (int)modal.y + 36, 52, (Color){250, 240, 205, 255});
                            DrawText("RESOLUTION", (int)modal.x + 72, (int)modal.y + 118, 34, (Color){200, 226, 255, 255});

                            if (IsOptionClicked("2560 x 1600", (int)modal.x + 88, (int)modal.y + 166, 30, WHITE, YELLOW))
                            {
                                ApplyWindowedResolution(2560, 1600, &camera);
                            }
                            if (IsOptionClicked("3200 x 2000", (int)modal.x + 88, (int)modal.y + 206, 30, WHITE, YELLOW))
                            {
                                ApplyWindowedResolution(3200, 2000, &camera);
                            }
                            if (IsOptionClicked("1920 x 1080", (int)modal.x + 88, (int)modal.y + 246, 30, WHITE, YELLOW))
                            {
                                ApplyWindowedResolution(1920, 1080, &camera);
                            }

                            {
                                char* fullscreenText = isFullscreen ? "WINDOWED MODE" : "FULLSCREEN MODE";
                                if (IsOptionClicked(fullscreenText, (int)modal.x + 88, (int)modal.y + 286, 30, WHITE, YELLOW))
                                {
                                    ToggleFullscreenMode();
                                }
                            }

                            DrawText(TextFormat("SCENE TEXT SPEED: %.0f cps", GetSceneTextSpeed()), (int)modal.x + 88, (int)modal.y + 324, 28, (Color){200, 226, 255, 255});
                            if (IsOptionClicked("SLOWER (-)", (int)modal.x + 88, (int)modal.y + 360, 26, WHITE, YELLOW))
                            {
                                SetSceneTextSpeed(GetSceneTextSpeed() - 6.0f);
                            }
                            if (IsOptionClicked("FASTER (+)", (int)modal.x + 252, (int)modal.y + 360, 26, WHITE, YELLOW))
                            {
                                SetSceneTextSpeed(GetSceneTextSpeed() + 6.0f);
                            }

                            if (IsOptionClicked("BACK", (int)modal.x + 458, (int)modal.y + 360, 34, WHITE, GREEN))
                            {
                                pauseSettingsPanel = false;
                            }
                        }

                        if (pauseMessageTimer > 0.0f)
                        {
                            DrawText(pauseMessage, (int)modal.x + 210, (int)modal.y + 388, 28, ORANGE);
                        }
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
    ShutdownOpeningFlowAssets();
    ShutdownMapSystem();
    CloseWindow();
    return 0;
}