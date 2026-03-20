# Galgame-Style Scene System Documentation

## Overview
Your game now has a complete scene/dialog system similar to visual novels (galgame). The system allows you to:
- Display text dialogs with character names
- Show background images
- Display character sprites
- Handle player input (Press ENTER to advance)
- Seamlessly integrate into your existing gameplay

## Architecture

The scene system consists of 3 main components:

### 1. **Scene.h / Scene.c** - Core Scene Engine
- Manages scene playback state
- Handles texture loading/unloading
- Renders dialog box and text
- Processes player input (ENTER key)
- Automatically wraps long text

### 2. **SceneData.h** - Scene Definition
- Contains scene data (dialog lines and metadata)
- Provides example scenes you can copy
- Template documentation for creating new scenes

### 3. **GameStates.h** - Updated Game State
- Added `STATE_SCENE` for scene playback
- Scenes interrupt gameplay when active
- Automatically returns to `STATE_GAMEPLAY` when done

---

## Quick Start: Creating Your First Scene

### Step 1: Define Your Scene Lines

In `Scene/SceneData.h`, add your scene:

```c
// Add this to SceneData.h
static SceneLine MyFirstSceneLines[] = {
    {
        .characterName = "Hero",
        .dialogText = "I sense danger ahead...",
        .backgroundPath = NULL,  // Skip for now
        .characterImagePath = NULL,
        .characterX = 400,
        .characterY = 300,
        .characterScale = 1.0f
    },
    {
        .characterName = "Hero",
        .dialogText = "I must be ready for anything!",
        .backgroundPath = NULL,
        .characterImagePath = NULL,
        .characterX = 400,
        .characterY = 300,
        .characterScale = 1.0f
    }
};
```

### Step 2: Create the SceneData Struct

```c
// Add this to SceneData.h after the lines array
static SceneData MyFirstScene = {
    .sceneName = "my_first_scene",
    .lines = MyFirstSceneLines,
    .lineCount = 2  // Must match number of lines above
};
```

### Step 3: Trigger the Scene in Gameplay

In `main.c`, within the `STATE_GAMEPLAY` case, add a trigger condition:

```c
case STATE_GAMEPLAY:
    // ... existing code ...
    
    // Example: Trigger scene when player reaches certain position
    if (plyr.pos.x > 5000 && plyr.pos.y < 1000) {
        InitScene(&currentScene, &MyFirstScene);
        currentScreen = STATE_SCENE;
    }
    
    // ... rest of gameplay code ...
    break;
```

---

## Scene Data Structure

### SceneLine Fields

| Field | Type | Description | Example |
|-------|------|-------------|---------|
| `characterName` | `const char*` | Name of speaking character | `"Commander"` |
| `dialogText` | `const char*` | Dialog text (use `\n` for breaks) | `"Attack now!\nDo not hesitate!"` |
| `backgroundPath` | `const char*` | Path to background image | `"assets/bg_forest.png"` or `NULL` |
| `characterImagePath` | `const char*` | Path to character sprite | `"assets/hero.png"` or `NULL` |
| `characterX` | `float` | Character X position (screen coords) | `400` |
| `characterY` | `float` | Character Y position (screen coords) | `300` |
| `characterScale` | `float` | Character sprite scale | `1.0f` (normal), `0.5f` (half), `2.0f` (double) |

### SceneData Fields

| Field | Type | Description |
|-------|------|-------------|
| `sceneName` | `const char*` | Unique identifier for scene |
| `lines` | `SceneLine*` | Pointer to lines array |
| `lineCount` | `int` | Total number of lines |

---

## Advanced Features

### Text Line Breaks

Use `\n` to break text into multiple lines:

```c
.dialogText = "This is line one.\nThis is line two.\nThis is line three."
```

### Multiple Characters

Each line can have different character images:

```c
static SceneLine ConversationLines[] = {
    {
        .characterName = "Alice",
        .dialogText = "What do you want?",
        .characterImagePath = "assets/alice.png",
        .characterX = 200, .characterY = 300, .characterScale = 1.0f
    },
    {
        .characterName = "Bob",
        .dialogText = "I need your help!",
        .characterImagePath = "assets/bob.png",
        .characterX = 500, .characterY = 300, .characterScale = 1.0f
    }
};
```

### Conditional Scenes

Trigger different scenes based on game state:

```c
if (plyr.health < 50) {
    // Low health - show warning scene
    InitScene(&currentScene, &WarningScene);
} else {
    // Normal scene
    InitScene(&currentScene, &NormalScene);
}
currentScreen = STATE_SCENE;
```

### Scene with Full Assets

For the best visual experience:

```c
.backgroundPath = "assets/backgrounds/throne_room.png",
.characterImagePath = "assets/characters/king.png"
```

---

## Image Preparation Tips

### Background Images
- **Resolution**: Match your game window (2560x1600 recommended)
- **Format**: PNG for transparency support
- **Size**: Should cover entire screen

### Character Sprites
- **Resolution**: Typically 600-800px tall
- **Format**: PNG with transparent background
- **Position**: Use characterX/Y to place them (e.g., left: 300, center: 1200, right: 2000)
- **Scale**: Adjust with characterScale if needed

---

## Special Controls

During scene playback:
- **ENTER**: Advance to next line / Close scene when finished
- Scene is **modal** - blocks normal gameplay input
- Returns to `STATE_GAMEPLAY` automatically upon completion

---

## Complete Example Scene

Here's a full, production-ready example:

```c
static SceneLine BossEncounterLines[] = {
    {
        .characterName = "Mysterious Voice",
        .dialogText = "So, a challenger approaches...",
        .backgroundPath = "assets/bg_boss_lair.png",
        .characterImagePath = NULL,
        .characterX = 0, .characterY = 0, .characterScale = 1.0f
    },
    {
        .characterName = "Boss",
        .dialogText = "Your journey ends here!",
        .backgroundPath = "assets/bg_boss_lair.png",  // Keep same background
        .characterImagePath = "assets/boss_character.png",
        .characterX = 1500, .characterY = 200, .characterScale = 1.2f
    },
    {
        .characterName = "Hero",
        .dialogText = "Not if I have anything to say about it!\nLet's dance!",
        .backgroundPath = "assets/bg_boss_lair.png",
        .characterImagePath = "assets/hero_character.png",
        .characterX = 300, .characterY = 300, .characterScale = 1.0f
    }
};

static SceneData BossEncounterScene = {
    .sceneName = "boss_encounter",
    .lines = BossEncounterLines,
    .lineCount = 3
};

// In main.c, trigger it:
// if (bossHealthZero && !bossScenePlayed) {
//     InitScene(&currentScene, &BossEncounterScene);
//     currentScreen = STATE_SCENE;
//     bossScenePlayed = true;
// }
```

---

## Customization

### Changing Dialog Box Appearance

In `Scene.c`, modify `DrawDialogBox()`:

```c
// Change box height (default 250):
scene->dialogBoxHeight = 300.0f;

// Change text color (default WHITE):
scene->textColor = YELLOW;

// Change box background (default dark semi-transparent):
scene->boxColor = (Color){50, 50, 100, 200};

// Change font size (default 24):
scene->fontSize = 28;
```

### Disabling Specific Elements

Make text-only dialogs:
```c
.backgroundPath = NULL,
.characterImagePath = NULL
```

---

## Implementation in Your Game

### Trigger Points
Good places to add scenes:
- Map transitions
- Enemy encounters
- Cutscenes
- Boss meetings
- Story progression events
- Quest interactions

### Example Trigger
```c
case STATE_GAMEPLAY:
    // ... existing update code ...
    
    // Check for scene triggers
    if (plyr.pos.x > 8000 && !encounterTriggered) {
        InitScene(&currentScene, &EnemyEncounterScene);
        currentScreen = STATE_SCENE;
        encounterTriggered = true;
    }
    
    // ... rest of code ...
    break;
```

---

## Common Patterns

### Quest Dialog
```c
.dialogText = "Find the ancient artifact in the eastern ruins.\nReport back when done."
```

### Multiple Scene Sequence
Create multiple SceneData structs and trigger them in sequence based on story progress.

### Dynamic Dialogs
Based on player actions, show different scenes:
```c
if (save_the_npc) {
    InitScene(&currentScene, &GratitudeScene);
} else {
    InitScene(&currentScene, &AngerScene);
}
```

---

## Troubleshooting

**Issue**: Scene doesn't display
- Check that `GameStates.h` includes `STATE_SCENE`
- Verify `main.c` includes `Scene.h` and `SceneData.h`
- Ensure you set `currentScreen = STATE_SCENE` in your trigger code

**Issue**: Text is cut off
- Dialog box height might be too small
- Increase `scene->dialogBoxHeight` in `Scene.c`
- Text wrapping is automatic for long lines

**Issue**: Images not showing
- Verify file paths are correct (relative to game folder)
- Use `PNG` format with transparency
- Check that images exist at the specified path
- Use `NULL` to skip missing images

**Issue**: Game compiles but scene won't trigger
- Verify the trigger condition is being checked
- Confirm you're setting both:
  1. `InitScene(&currentScene, &YourScene);`
  2. `currentScreen = STATE_SCENE;`

---

## Next Steps

1. Prepare your background images (resolution: 2560x1600)
2. Create character sprites PNG files
3. Add scenes to `SceneData.h` following the template
4. Add trigger conditions in `main.c`
5. Test by reaching the trigger conditions in gameplay

Happy storytelling! 🎮
