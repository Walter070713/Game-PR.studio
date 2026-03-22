#ifndef SCENE_DATA_H
#define SCENE_DATA_H

#include "Scene.h"

// ============== EXAMPLE OPENING SCENE ==============
static SceneLine OpeningSceneLines[] = {
    {
        .characterName = "Narrator",
        .dialogText = "Mission briefing begins now.",
        .backgroundPath ="Assets/backgrounds/RONDYS OFFICE_.png",
        .characterImagePath = "Assets/characters/Main Character/Characters.png",
        .characterX = 0,
        .characterY = 0,
        .characterScale = 1.0f
    },
    {
        .characterName = "Commander",
        .dialogText = "Rondy, wake up. We lost contact with the forward team.",
        .backgroundPath = NULL,
        .characterImagePath = NULL,
        .characterX = 400,
        .characterY = 300,
        .characterScale = 1.0f
    },
    {
        .characterName = "Rondy",
        .dialogText = "I am moving now. First checkpoint: exit the prep room.",
        .backgroundPath = NULL,
        .characterImagePath = NULL,
        .characterX = 400,
        .characterY = 300,
        .characterScale = 1.0f
    },
    {
        .characterName = "Commander",
        .dialogText = "Use the marked door. Press E when you are close.",
        .backgroundPath = NULL,
        .characterImagePath = NULL,
        .characterX = 400,
        .characterY = 300,
        .characterScale = 1.0f
    }
};

static SceneData OpeningScene = {
    .sceneName = "opening",
    .lines = OpeningSceneLines,
    .lineCount = 4
};

// ============== HOW TO CREATE YOUR OWN SCENES ==============
/*

Step 1: Create a SceneLine array with designated initializers:
   static SceneLine MySceneLines[] = {
       {
           .characterName = "Character Name",
           .dialogText = "Dialog text here. Use \\n for line breaks.",
           .backgroundPath = "assets/bg_forest.png",    // Background image path (or NULL)
           .characterImagePath = "assets/char_main.png", // Character sprite path (or NULL) 
           .characterX = 400,   // Character X position
           .characterY = 300,   // Character Y position
           .characterScale = 1.0f // Character scale (1.0 = normal size)
       },
       // ... more lines
   };

Step 2: Create a SceneData struct
   static SceneData MyScene = {
       .sceneName = "my_scene_id",
       .lines = MySceneLines,
       .lineCount = 1  // Number of lines in array
   };

Step 3: Trigger the scene from your game
   // When you want to start the scene:
   InitScene(&currentScene, &MyScene);
   currentScreen = STATE_SCENE;
   
Step 4: The scene handling is automatic
   // In your STATE_SCENE update: UpdateScene handles input and advancement
   // In your STATE_SCENE draw: DrawScene renders everything
   // When done, automatically returns to STATE_GAMEPLAY

QUICK REFERENCE:
- dialogText: Use \n for line breaks
- backgroundPath: NULL to skip, "path/to/image.png" to load
- characterImagePath: NULL to skip, "path/to/sprite.png" to load
- characterX/Y: Position on screen (top-left is 0,0)
- characterScale: 1.0 = normal, 0.5 = half size, 2.0 = double size
- Best practices: 
  * Background images at screen resolution (2560x1600 for your game)
  * Character sprites with transparent backgrounds (PNG format)
  * Keep text concise, use line breaks for readability

EXAMPLE TRIGGER IN GAMEPLAY:
   // Add this somewhere in STATE_GAMEPLAY section when a condition is met:
   if (plyr.pos.x > 1000 && plyr.pos.y < 500) {
       InitScene(&currentScene, &MyScene);  // Start the scene
       currentScreen = STATE_SCENE;         // Switch to scene state
   }

*/

#endif
