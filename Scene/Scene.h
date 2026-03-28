#ifndef SCENE_H
#define SCENE_H

#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>

// Scene line structure - contains all data for a single dialog line
typedef struct {
    const char* characterName;      // Name of character speaking
    const char* dialogText;         // The dialog text to display
    const char* backgroundPath;     // Path to background image (or NULL for none)
    const char* characterImagePath; // Path to character sprite (or NULL for none)
    float characterX;               // X position of character sprite
    float characterY;               // Y position of character sprite
    float characterScale;           // Scale of character sprite
} SceneLine;

// Complete scene - multiple dialog lines
typedef struct {
    const char* sceneName;          // Identifier for this scene
    SceneLine* lines;               // Array of dialog lines
    int lineCount;                  // Total number of lines
} SceneData;

// Scene state - tracks current playthrough
typedef struct {
    bool isActive;                  // Is scene currently playing?
    int currentLineIndex;           // Which line are we on?
    SceneData* data;                // Pointer to scene data
    
    // Loaded textures (cached during scene)
    Texture2D backgroundTexture;
    Texture2D characterTexture;
    bool bgLoaded;
    bool charLoaded;
    
    // UI parameters
    float dialogBoxHeight;          // Height of dialog box at bottom
    int fontSize;                   // Font size for dialog
    Color textColor;                // Text color for dialog
    Color boxColor;                 // Background color of dialog box

    // Typewriter effect state
    int visibleCharCount;           // Number of currently visible chars in active line
    float typingCharsPerSecond;     // Typewriter speed
    float typingAccumulator;        // Fractional char accumulator
    
    // Background animation
    float backgroundScrollTime;     // Time for background parallax effect
} Scene;

/**
 * Load scene/dialog lines from an external text file.
 *
 * File format (one entry per line, '|' separated):
 * characterName|dialogText|backgroundPath|characterImagePath|characterX|characterY|characterScale
 *
 * Notes:
 * - Use NONE (or empty field) for backgroundPath/characterImagePath when not used.
 * - Use \n inside dialogText for manual line breaks.
 */
bool LoadSceneDataFromFile(const char* sceneName, const char* filePath, SceneData* outScene);

/**
 * Release memory allocated by LoadSceneDataFromFile.
 */
void UnloadSceneData(SceneData* sceneData);

/**
 * Read or update the global scene typewriter speed (characters per second).
 * New scenes read this value during InitScene.
 */
float GetSceneTextSpeed(void);
void SetSceneTextSpeed(float charsPerSecond);

// ============== FUNCTION DECLARATIONS ==============

/**
 * Initialize a scene with data
 * @param scene - Scene state to initialize
 * @param sceneData - Scene data to play
 */
void InitScene(Scene* scene, SceneData* sceneData);

/**
 * Update scene state (handle input, advance lines)
 * @param scene - Scene state to update
 * @return true if scene is still playing, false if finished
 */
bool UpdateScene(Scene* scene);

/**
 * Render the scene (background, character, dialog box, text)
 * @param scene - Scene to render
 */
void DrawScene(Scene* scene);

/**
 * Clean up scene resources (unload textures)
 * @param scene - Scene to clean up
 */
void CleanupScene(Scene* scene);

#endif
