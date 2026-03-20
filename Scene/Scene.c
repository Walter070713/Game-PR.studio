#include "Scene.h"
#include <stdio.h>

// Internal helper functions
static void LoadSceneTextures(Scene* scene);
static void UnloadSceneTextures(Scene* scene);
static void DrawDialogBox(Scene* scene);
static void DrawText_Wrapped(const char* text, float x, float y, int fontSize, Color color, float maxWidth);

void InitScene(Scene* scene, SceneData* sceneData) {
    if (!scene || !sceneData) return;
    
    scene->isActive = true;
    scene->currentLineIndex = 0;
    scene->data = sceneData;
    
    scene->backgroundTexture = (Texture2D){0};
    scene->characterTexture = (Texture2D){0};
    scene->bgLoaded = false;
    scene->charLoaded = false;
    
    // Default UI settings
    scene->dialogBoxHeight = 250.0f;
    scene->fontSize = 24;
    scene->textColor = WHITE;
    scene->boxColor = (Color){40, 40, 40, 220};  // Semi-transparent dark
    
    // Load textures for first line
    LoadSceneTextures(scene);
}

bool UpdateScene(Scene* scene) {
    if (!scene || !scene->isActive) return false;
    
    // Check if Enter key is pressed to advance line
    if (IsKeyPressed(KEY_ENTER)) {
        if (!AdvanceSceneLine(scene)) {
            // Scene is complete
            scene->isActive = false;
            UnloadSceneTextures(scene);
            return false;
        }
    }
    
    return true;
}

void DrawScene(Scene* scene) {
    if (!scene || !scene->isActive) return;
    
    // Draw background if available
    if (scene->bgLoaded && scene->backgroundTexture.id != 0) {
        DrawTexture(scene->backgroundTexture, 0, 0, WHITE);
    }
    
    // Get current line
    if (scene->currentLineIndex < scene->data->lineCount) {
        SceneLine* currentLine = &scene->data->lines[scene->currentLineIndex];
        
        // Draw character sprite if available
        if (scene->charLoaded && scene->characterTexture.id != 0) {
            Vector2 charPos = {currentLine->characterX, currentLine->characterY};
            DrawTextureEx(scene->characterTexture, charPos, 0.0f, currentLine->characterScale, WHITE);
        }
    }
    
    // Draw dialog box and text
    DrawDialogBox(scene);
}

void CleanupScene(Scene* scene) {
    if (!scene) return;
    
    UnloadSceneTextures(scene);
    scene->isActive = false;
    scene->data = NULL;
}

bool AdvanceSceneLine(Scene* scene) {
    if (!scene || !scene->data) return false;
    
    scene->currentLineIndex++;
    
    if (scene->currentLineIndex >= scene->data->lineCount) {
        // Scene is complete
        return false;
    }
    
    // Load textures for new current line
    LoadSceneTextures(scene);
    return true;
}

bool IsSceneComplete(Scene* scene) {
    if (!scene) return true;
    return scene->currentLineIndex >= scene->data->lineCount;
}

void SkipScene(Scene* scene) {
    if (!scene || !scene->data) return;
    
    scene->currentLineIndex = scene->data->lineCount;
    scene->isActive = false;
    UnloadSceneTextures(scene);
}

// ============== INTERNAL HELPER FUNCTIONS ==============

static void LoadSceneTextures(Scene* scene) {
    if (!scene || !scene->data || scene->currentLineIndex >= scene->data->lineCount) return;
    
    SceneLine* currentLine = &scene->data->lines[scene->currentLineIndex];
    
    // Unload old textures if they exist
    if (scene->bgLoaded && scene->backgroundTexture.id != 0) {
        UnloadTexture(scene->backgroundTexture);
        scene->bgLoaded = false;
    }
    if (scene->charLoaded && scene->characterTexture.id != 0) {
        UnloadTexture(scene->characterTexture);
        scene->charLoaded = false;
    }
    
    // Load background
    if (currentLine->backgroundPath != NULL) {
        if (FileExists(currentLine->backgroundPath)) {
            scene->backgroundTexture = LoadTexture(currentLine->backgroundPath);
            scene->bgLoaded = true;
        }
    }
    
    // Load character image
    if (currentLine->characterImagePath != NULL) {
        if (FileExists(currentLine->characterImagePath)) {
            scene->characterTexture = LoadTexture(currentLine->characterImagePath);
            scene->charLoaded = true;
        }
    }
}

// Unload scene
static void UnloadSceneTextures(Scene* scene) {
    if (!scene) return;
    
    if (scene->bgLoaded && scene->backgroundTexture.id != 0) {
        UnloadTexture(scene->backgroundTexture);
        scene->bgLoaded = false;
    }
    if (scene->charLoaded && scene->characterTexture.id != 0) {
        UnloadTexture(scene->characterTexture);
        scene->charLoaded = false;
    }
}

static void DrawDialogBox(Scene* scene) {
    if (!scene || !scene->data || scene->currentLineIndex >= scene->data->lineCount) return;
    
    SceneLine* currentLine = &scene->data->lines[scene->currentLineIndex];
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // Draw semi-transparent dialog box at bottom
    float boxY = screenHeight - scene->dialogBoxHeight;
    DrawRectangle(0, (int)boxY, screenWidth, (int)scene->dialogBoxHeight, scene->boxColor);
    DrawRectangleLines(0, (int)boxY, screenWidth, (int)scene->dialogBoxHeight, YELLOW);
    
    // Draw character name
    float nameX = 20.0f;
    float nameY = boxY + 10.0f;
    DrawText(currentLine->characterName, (int)nameX, (int)nameY, 28, YELLOW);
    
    // Draw dialog text with wrapping
    float textX = 20.0f;
    float textY = boxY + 45.0f;
    float maxTextWidth = screenWidth - 40.0f;
    DrawText_Wrapped(currentLine->dialogText, textX, textY, scene->fontSize, scene->textColor, maxTextWidth);
    
    // Draw "Press ENTER to continue" hint
    const char* continueText = "Press ENTER to continue...";
    int continueWidth = MeasureText(continueText, 18);
    DrawText(continueText, screenWidth - continueWidth - 20, (int)(boxY + scene->dialogBoxHeight - 35), 18, GRAY);
}

// Text wrapping helper - draws text with line breaks to fit within maxWidth
static void DrawText_Wrapped(const char* text, float x, float y, int fontSize, Color color, float maxWidth) {
    if (!text) return;
    
    Vector2 position = {x, y};
    int charIndex = 0;
    char buffer[256];
    int bufferIndex = 0;
    
    while (text[charIndex] != '\0') {
        char currentChar = text[charIndex];
        
        if (currentChar == '\n') {
            // Handle explicit newline
            buffer[bufferIndex] = '\0';
            DrawText(buffer, (int)position.x, (int)position.y, fontSize, color);
            position.y += fontSize + 5;
            bufferIndex = 0;
            charIndex++;
            continue;
        }
        
        // Add character to buffer
        buffer[bufferIndex++] = currentChar;
        buffer[bufferIndex] = '\0';
        
        // Check if line is too wide
        int lineWidth = MeasureText(buffer, fontSize);
        if (lineWidth > maxWidth && bufferIndex > 1) {
            // Remove last character and draw line
            buffer[bufferIndex - 1] = '\0';
            DrawText(buffer, (int)position.x, (int)position.y, fontSize, color);
            position.y += fontSize + 5;
            
            // Start new line with current character
            buffer[0] = currentChar;
            buffer[1] = '\0';
            bufferIndex = 1;
        }
        
        charIndex++;
    }
    
    // Draw remaining text
    if (bufferIndex > 0) {
        buffer[bufferIndex] = '\0';
        DrawText(buffer, (int)position.x, (int)position.y, fontSize, color);
    }
}
