#ifndef SCENE_DATA_H
#define SCENE_DATA_H

#include "Scene.h"

// ============== OPENING SCENE ==============
// All lines belong to one scene: Line 1 = Scene 1, Line 2 = Scene 2, Line 3 = Scene 3, Line 4 = Scene 4, Line 5 = Scene 5, Line 6 = Scene 6
static SceneLine OpeningWakeNarrationLines[] = {
   {
      .characterName = "Narration",
      .dialogText = "(You are lying on a bed,just wake up)",
      .backgroundPath = NULL,
      .characterImagePath = NULL,
      .characterX = 0,
      .characterY = 0,
      .characterScale = 1.0f
   },
   {
      .characterName = "Player",
      .dialogText = "Where...where am I...",
      .backgroundPath = NULL,
      .characterImagePath = NULL,
      .characterX = 0,
      .characterY = 0,
      .characterScale = 1.0f
   },
   {
      .characterName = "Narration",
      .dialogText = "(You sit up slowly. Look around. Nothing in the room is personal. )",
      .backgroundPath = "Assets\\backgrounds\\3-SERVER ROOM CHAIR DESK.png",
      .characterImagePath = NULL,
      .characterX = 0,
      .characterY = 0,
      .characterScale = 1.0f
   },
   {
      .characterName = "Narration",
      .dialogText = "(No posters. No photos. Just a desk, a chair, like a render that hasn't fully loaded. )",
      .backgroundPath = "Assets\\backgrounds\\3-SERVER ROOM CHAIR DESK.png",
      .characterImagePath = NULL,
      .characterX = 0,
      .characterY = 0,
      .characterScale = 1.0f
   },
   {
      .characterName = "Narration",
      .dialogText = "(You notice something. On the desk, a single sheet of paper. Handwritten:)",
      .backgroundPath = "Assets\\backgrounds\\3-SERVER ROOM CHAIR DESK.png",
      .characterImagePath = NULL,
      .characterX = 0,
      .characterY = 0,
      .characterScale = 1.0f
   },
   {
      .characterName = "Paper",
      .dialogText = "You don't remember me. That's okay. You're not supposed to. Come find me when you're ready.\nps. Rondy",
      .backgroundPath = "Assets\\backgrounds\\3-SERVER ROOM CHAIR DESK.png",
      .characterImagePath = NULL,
      .characterX = 0,
      .characterY = 0,
      .characterScale = 1.0f
   }
};

static SceneData OpeningWakeNarrationScene = {
   .sceneName = "opening_wake_narration",
   .lines = OpeningWakeNarrationLines,
   .lineCount = 6
};

// ============== RONDY OFFICE SCENE ==============
static SceneLine RondyOfficeIntroLines[] = {
   {
      .characterName = "Rondy",
      .dialogText = "Ah. You're awake. Good. I wasn't sure which version would... never mind.",
      .backgroundPath = "Assets\\backgrounds\\RONDYS OFFICE_.png",
      .characterImagePath = "Assets\\characters\\Rondy Intro Video\\Rondy Default.png",
      .characterX = -600,
      .characterY = 460,
      .characterScale = 1.0f
   },
   {
      .characterName = "Rondy",
      .dialogText = "Coffee?",
      .backgroundPath = "Assets\\backgrounds\\RONDYS OFFICE_.png",
      .characterImagePath = "Assets\\characters\\Rondy Intro Video\\Rondy Coffe.png",
      .characterX = -600,
      .characterY = 460,
      .characterScale = 1.0f
   },
   {
      .characterName = "Rondy",
      .dialogText = "**slurp**",
      .backgroundPath = "Assets\\backgrounds\\RONDYS OFFICE_.png",
      .characterImagePath = "Assets\\characters\\Rondy Intro Video\\Randy Slurp the coffe.png",
      .characterX = -600,
      .characterY = 460,
      .characterScale = 1.0f
   },
   {
      .characterName = "Rondy",
      .dialogText = "You're probably wondering who you are. Where you came from. Why your dorm room looks like that.",
      .backgroundPath = "Assets\\backgrounds\\RONDYS OFFICE_.png",
      .characterImagePath = "Assets\\characters\\Rondy Intro Video\\Rondy Coffe.png",
      .characterX = -600,
      .characterY = 460,
      .characterScale = 1.0f
   },
   {
      .characterName = "Rondy",
      .dialogText = "Here's the thing. I don't know either. Not all of it.",
      .backgroundPath = "Assets\\backgrounds\\RONDYS OFFICE_.png",
      .characterImagePath = "Assets\\characters\\Rondy Intro Video\\Rondy Default.png",
      .characterX = -600,
      .characterY = 460,
      .characterScale = 1.0f
   },
   {
      .characterName = "Rondy",
      .dialogText = "I found you three weeks ago, wandering around the CS building at 3 AM, staring blankly at the wall.",
      .backgroundPath = "Assets\\backgrounds\\1-SERVER ROOM.png",
      .characterImagePath = "Assets\\characters\\Rondy Intro Video\\Rondy Default.png",
      .characterX = -600,
      .characterY = 460,
      .characterScale = 1.0f
   },
   {
      .characterName = "Rondy",
      .dialogText = "You couldn't tell me your name. Couldn't tell me where you lived. So I brought you here. Got you enrolled.",
      .backgroundPath = "Assets\\backgrounds\\1-SERVER ROOM.png",
      .characterImagePath = "Assets\\characters\\Rondy Intro Video\\Rondy Default.png",
      .characterX = -600,
      .characterY = 460,
      .characterScale = 1.0f
   },
   {
      .characterName = "Rondy",
      .dialogText = "Before I tell you what is exactly happening out there, let me teach you some integral SKILLS.",
      .backgroundPath = "Assets\\backgrounds\\RONDYS OFFICE_.png",
      .characterImagePath = "Assets\\characters\\Rondy Intro Video\\Rondy Default.png",
      .characterX = -600,
      .characterY = 460,
      .characterScale = 1.0f
   },
   {
      .characterName = "Rondy",
      .dialogText = "Meet you at the tutorial room!",
      .backgroundPath = "Assets\\backgrounds\\RONDYS OFFICE_.png",
      .characterImagePath = "Assets\\characters\\Rondy Intro Video\\Rondy Default.png",
      .characterX = -600,
      .characterY = 460,
      .characterScale = 1.0f
   }
};

static SceneData RondyOfficeIntroScene = {
   .sceneName = "rondy_office_intro",
   .lines = RondyOfficeIntroLines,
   .lineCount = 9
};

// All opening scene dialogs removed - replaced with in-game NPC interaction

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
