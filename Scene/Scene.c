#include "Scene.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Internal helper functions
static void LoadSceneTextures(Scene* scene);
static void UnloadSceneTextures(Scene* scene);
static void DrawDialogBox(Scene* scene);
static void DrawText_Wrapped(const char* text, float x, float y, int fontSize, Color color, float maxWidth, int maxVisibleChars, bool centerHorizontal);
static int GetLineTextLength(const Scene* scene);
static bool SamePath(const char* a, const char* b);

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
    scene->fontSize = 38;
    scene->textColor = WHITE;
    scene->boxColor = (Color){40, 40, 40, 220};  // Semi-transparent dark

    // Typewriter defaults
    scene->visibleCharCount = 0;
    scene->typingCharsPerSecond = 42.0f;
    scene->typingAccumulator = 0.0f;
    
    // Background animation
    scene->backgroundScrollTime = 0.0f;
    
    // Load textures for first line
    LoadSceneTextures(scene);
}

bool UpdateScene(Scene* scene) {
    if (!scene || !scene->isActive) return false;

    // Update background animation
    scene->backgroundScrollTime += GetFrameTime();
    
    // Reveal text gradually for the active line.
    {
        int lineLen = GetLineTextLength(scene);
        if (scene->visibleCharCount < lineLen)
        {
            scene->typingAccumulator += scene->typingCharsPerSecond * GetFrameTime();
            while (scene->typingAccumulator >= 1.0f && scene->visibleCharCount < lineLen)
            {
                scene->visibleCharCount++;
                scene->typingAccumulator -= 1.0f;
            }
        }
    }
    
    // Enter: if line is still typing, complete it instantly; otherwise advance.
    if (IsKeyPressed(KEY_ENTER)) {
        int lineLen = GetLineTextLength(scene);
        if (scene->visibleCharCount < lineLen)
        {
            scene->visibleCharCount = lineLen;
            scene->typingAccumulator = 0.0f;
        }
        else if (!AdvanceSceneLine(scene)) {
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
    
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    const float baseWidth = 2560.0f;
    const float baseHeight = 1600.0f;
    const float ratioX = (float)screenWidth / baseWidth;
    const float ratioY = (float)screenHeight / baseHeight;
    const float contentScale = (ratioX < ratioY) ? ratioX : ratioY;

    // Draw background if available (full-screen) with zoom and smooth scrolling
    if (scene->bgLoaded && scene->backgroundTexture.id != 0) {
        float scrollX = sinf(scene->backgroundScrollTime * 0.36f) * 52.0f;
        float scrollY = cosf(scene->backgroundScrollTime * 0.28f) * 12.0f;
        float zoom = 1.14f;
        float dw = (float)screenWidth * zoom;
        float dh = (float)screenHeight * zoom;
        float dx = -((dw - (float)screenWidth) * 0.5f) + scrollX;
        float dy = -((dh - (float)screenHeight) * 0.5f) + scrollY;
        Rectangle source = {0, 0, (float)scene->backgroundTexture.width, (float)scene->backgroundTexture.height};
        Rectangle dest = {dx, dy, dw, dh};
        Vector2 origin = {0, 0};
        DrawTexturePro(scene->backgroundTexture, source, dest, origin, 0.0f, WHITE);
    }
    
    // Get current line
    if (scene->currentLineIndex < scene->data->lineCount) {
        SceneLine* currentLine = &scene->data->lines[scene->currentLineIndex];
        
        // Draw character sprite if available.
        // Position is driven by SceneData characterX/characterY for per-scene tuning.
        if (scene->charLoaded && scene->characterTexture.id != 0) {
            float portraitH = (float)screenHeight * 0.78f * currentLine->characterScale;
            float aspect = (float)scene->characterTexture.width / (float)scene->characterTexture.height;
            float portraitW = portraitH * aspect;

            float dw = portraitW;
            float dh = portraitH;
            float dx = currentLine->characterX * ratioX;
            float dy = currentLine->characterY * ratioY;

            Rectangle source = {0, 0, (float)scene->characterTexture.width, (float)scene->characterTexture.height};
            Rectangle dest = {dx, dy, dw, dh};
            Vector2 origin = {0, 0};
            DrawTexturePro(scene->characterTexture, source, dest, origin, 0.0f, WHITE);
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

    scene->visibleCharCount = 0;
    scene->typingAccumulator = 0.0f;
    
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
    const SceneLine* previousLine = NULL;
    if (scene->currentLineIndex > 0)
    {
        previousLine = &scene->data->lines[scene->currentLineIndex - 1];
    }

    bool keepBackground = scene->bgLoaded && previousLine &&
        SamePath(previousLine->backgroundPath, currentLine->backgroundPath);
    bool keepCharacter = scene->charLoaded && previousLine &&
        SamePath(previousLine->characterImagePath, currentLine->characterImagePath);
    
    // Unload old textures if they exist
    if (!keepBackground && scene->bgLoaded && scene->backgroundTexture.id != 0) {
        UnloadTexture(scene->backgroundTexture);
        scene->bgLoaded = false;
    }
    if (!keepCharacter && scene->charLoaded && scene->characterTexture.id != 0) {
        UnloadTexture(scene->characterTexture);
        scene->charLoaded = false;
    }
    
    // Load background
    if (!scene->bgLoaded && currentLine->backgroundPath != NULL) {
        if (FileExists(currentLine->backgroundPath)) {
            scene->backgroundTexture = LoadTexture(currentLine->backgroundPath);
            scene->bgLoaded = true;
        }
    }
    
    // Load character image
    if (!scene->charLoaded && currentLine->characterImagePath != NULL) {
        if (FileExists(currentLine->characterImagePath)) {
            scene->characterTexture = LoadTexture(currentLine->characterImagePath);
            scene->charLoaded = true;
        }
    }
}

static bool SamePath(const char* a, const char* b)
{
    if (!a && !b) return true;
    if (!a || !b) return false;
    return strcmp(a, b) == 0;
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
    float nameY = boxY + 5.0f;
    DrawText(currentLine->characterName, (int)nameX, (int)nameY, 52, YELLOW);
    
    // Draw dialog text with wrapping - centered both horizontally and vertically in dialog box
    float textX = 20.0f;
    float textY = boxY + (scene->dialogBoxHeight * 0.5f) - (scene->fontSize * 0.5f);
    float maxTextWidth = screenWidth - 40.0f;
    DrawText_Wrapped(currentLine->dialogText, textX, textY, scene->fontSize, scene->textColor, maxTextWidth, scene->visibleCharCount, true);
    
    // Draw "Press ENTER to continue" hint
    int lineLen = GetLineTextLength(scene);
    const char* continueText = (scene->visibleCharCount < lineLen) ? "Press ENTER to skip typing..." : "Press ENTER to continue...";
    int continueWidth = MeasureText(continueText, 24);
    DrawText(continueText, screenWidth - continueWidth - 20, (int)(boxY + scene->dialogBoxHeight - 40), 24, YELLOW);
}

// Text wrapping helper - draws text with line breaks to fit within maxWidth
static void DrawText_Wrapped(const char* text, float x, float y, int fontSize, Color color, float maxWidth, int maxVisibleChars, bool centerHorizontal) {
    if (!text) return;
    
    Vector2 position = {x, y};
    int charIndex = 0;
    char buffer[256];
    int bufferIndex = 0;
    
    while (text[charIndex] != '\0' && charIndex < maxVisibleChars) {
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

static int GetLineTextLength(const Scene* scene)
{
    if (!scene || !scene->data || scene->currentLineIndex >= scene->data->lineCount) return 0;
    const SceneLine* currentLine = &scene->data->lines[scene->currentLineIndex];
    if (!currentLine || !currentLine->dialogText) return 0;
    return (int)strlen(currentLine->dialogText);
}
