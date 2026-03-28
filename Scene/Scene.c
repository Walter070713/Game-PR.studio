#include "Scene.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

// Scene subsystem: data-file parsing + runtime scene playback (typewriter UI).
// Internal helper functions
static void LoadSceneTextures(Scene* scene);
static void UnloadSceneTextures(Scene* scene);
static void DrawDialogBox(Scene* scene);
static void DrawText_Wrapped(const char* text, float x, float y, int fontSize, Color color, float maxWidth, int maxVisibleChars);
static int GetLineTextLength(const Scene* scene);
static bool SamePath(const char* a, const char* b);
static bool AdvanceSceneLine(Scene* scene);
static char* CloneString(const char* source);
static char* TrimWhitespace(char* text);
static bool EqualsIgnoreCase(const char* a, const char* b);
static bool IsNullField(const char* field);
static void UnescapeEscapedNewlines(char* text);
static void FreeSceneLines(SceneLine* lines, int count);
static bool ParseSceneDataLine(char* rawLine, SceneLine* outLine);

// Global typewriter speed constraints used by settings screen.
static const float kMinSceneTextSpeed = 12.0f;
static const float kMaxSceneTextSpeed = 120.0f;
static float gSceneTypingSpeed = 42.0f;

// Read current global scene text speed.
float GetSceneTextSpeed(void)
{
    return gSceneTypingSpeed;
}

// Clamp and store global scene text speed.
void SetSceneTextSpeed(float charsPerSecond)
{
    if (charsPerSecond < kMinSceneTextSpeed) charsPerSecond = kMinSceneTextSpeed;
    if (charsPerSecond > kMaxSceneTextSpeed) charsPerSecond = kMaxSceneTextSpeed;
    gSceneTypingSpeed = charsPerSecond;
}

// Load a scene script file into heap-allocated SceneData.
bool LoadSceneDataFromFile(const char* sceneName, const char* filePath, SceneData* outScene)
{
    char* fileText;
    SceneLine* lines;
    int capacity = 16;
    int lineCount = 0;

    if (!sceneName || !filePath || !outScene) return false;

    fileText = LoadFileText(filePath);
    if (!fileText) return false;

    lines = (SceneLine*)calloc((size_t)capacity, sizeof(SceneLine));
    if (!lines)
    {
        UnloadFileText(fileText);
        return false;
    }

    {
        char* rawLine = strtok(fileText, "\r\n");
        while (rawLine)
        {
            char* line = TrimWhitespace(rawLine);

            if (line[0] != '\0' && line[0] != '#')
            {
                if (lineCount >= capacity)
                {
                    int newCapacity = capacity * 2;
                    SceneLine* expanded = (SceneLine*)realloc(lines, (size_t)newCapacity * sizeof(SceneLine));
                    if (!expanded)
                    {
                        FreeSceneLines(lines, lineCount);
                        free(lines);
                        UnloadFileText(fileText);
                        return false;
                    }

                    memset(expanded + capacity, 0, (size_t)(newCapacity - capacity) * sizeof(SceneLine));
                    lines = expanded;
                    capacity = newCapacity;
                }

                if (!ParseSceneDataLine(line, &lines[lineCount]))
                {
                    FreeSceneLines(lines, lineCount);
                    free(lines);
                    UnloadFileText(fileText);
                    return false;
                }

                lineCount++;
            }

            rawLine = strtok(NULL, "\r\n");
        }
    }

    // fileText buffer is no longer needed after tokenization/parsing.
    UnloadFileText(fileText);

    if (lineCount <= 0)
    {
        free(lines);
        return false;
    }

    outScene->sceneName = CloneString(sceneName);
    if (!outScene->sceneName)
    {
        FreeSceneLines(lines, lineCount);
        free(lines);
        return false;
    }

    outScene->lines = lines;
    outScene->lineCount = lineCount;
    return true;
}

// Free all heap memory owned by a SceneData loaded from file.
void UnloadSceneData(SceneData* sceneData)
{
    if (!sceneData) return;

    if (sceneData->lines)
    {
        FreeSceneLines(sceneData->lines, sceneData->lineCount);
        free(sceneData->lines);
    }

    free((void*)sceneData->sceneName);

    sceneData->sceneName = NULL;
    sceneData->lines = NULL;
    sceneData->lineCount = 0;
}

// Initialize one runtime scene playthrough from prepared SceneData.
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
    scene->typingCharsPerSecond = gSceneTypingSpeed;
    scene->typingAccumulator = 0.0f;
    
    // Background animation
    scene->backgroundScrollTime = 0.0f;
    
    // Load textures for first line
    LoadSceneTextures(scene);
}

// Advance scene animation/input and handle line progression.
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
            CleanupScene(scene);
            return false;
        }
    }
    
    return true;
}

// Render active scene background, portrait, and dialog box.
void DrawScene(Scene* scene) {
    if (!scene || !scene->isActive) return;
    
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    const float baseWidth = 2560.0f;
    const float baseHeight = 1600.0f;
    const float ratioX = (float)screenWidth / baseWidth;
    const float ratioY = (float)screenHeight / baseHeight;

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

// Stop active scene and release loaded textures.
void CleanupScene(Scene* scene) {
    if (!scene) return;
    
    UnloadSceneTextures(scene);
    scene->isActive = false;
    scene->data = NULL;
}

// Move to next scene line and refresh per-line textures.
static bool AdvanceSceneLine(Scene* scene) {
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

// ============== INTERNAL HELPER FUNCTIONS ==============

// Load textures needed by current line, reusing previous line assets when possible.
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

// Compare two optional file paths for equality.
static bool SamePath(const char* a, const char* b)
{
    if (!a && !b) return true;
    if (!a || !b) return false;
    return strcmp(a, b) == 0;
}

// Release currently loaded scene textures.
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

// Draw dialog frame, speaker name, wrapped text, and continue hint.
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
    DrawText_Wrapped(currentLine->dialogText, textX, textY, scene->fontSize, scene->textColor, maxTextWidth, scene->visibleCharCount);
    
    // Draw "Press ENTER to continue" hint
    int lineLen = GetLineTextLength(scene);
    const char* continueText = (scene->visibleCharCount < lineLen) ? "Press ENTER to skip typing..." : "Press ENTER to continue...";
    int continueWidth = MeasureText(continueText, 24);
    DrawText(continueText, screenWidth - continueWidth - 20, (int)(boxY + scene->dialogBoxHeight - 40), 24, YELLOW);
}

// Draw wrapped text with optional manual newlines and typing character limit.
static void DrawText_Wrapped(const char* text, float x, float y, int fontSize, Color color, float maxWidth, int maxVisibleChars) {
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

// Return full text length for current line.
static int GetLineTextLength(const Scene* scene)
{
    if (!scene || !scene->data || scene->currentLineIndex >= scene->data->lineCount) return 0;
    const SceneLine* currentLine = &scene->data->lines[scene->currentLineIndex];
    if (!currentLine || !currentLine->dialogText) return 0;
    return (int)strlen(currentLine->dialogText);
}

// Heap duplicate helper for scene script string fields.
static char* CloneString(const char* source)
{
    size_t length;
    char* copy;

    if (!source) return NULL;

    length = strlen(source);
    copy = (char*)malloc(length + 1);
    if (!copy) return NULL;

    memcpy(copy, source, length + 1);
    return copy;
}

// Trim leading and trailing whitespace in-place.
static char* TrimWhitespace(char* text)
{
    char* end;

    if (!text) return text;

    while (*text && isspace((unsigned char)*text)) text++;
    if (*text == '\0') return text;

    end = text + strlen(text) - 1;
    while (end >= text && isspace((unsigned char)*end))
    {
        *end = '\0';
        end--;
    }

    return text;
}

// Case-insensitive string equality helper.
static bool EqualsIgnoreCase(const char* a, const char* b)
{
    if (!a || !b) return false;

    while (*a && *b)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

// Treat empty/NONE/NULL markers as absent optional fields.
static bool IsNullField(const char* field)
{
    if (!field) return true;
    if (field[0] == '\0') return true;
    if (strcmp(field, "-") == 0) return true;
    if (EqualsIgnoreCase(field, "none")) return true;
    if (EqualsIgnoreCase(field, "null")) return true;
    return false;
}

// Convert escaped literals (\\n, \\\\) to runtime string characters.
static void UnescapeEscapedNewlines(char* text)
{
    char* readCursor;
    char* writeCursor;

    if (!text) return;

    readCursor = text;
    writeCursor = text;

    while (*readCursor)
    {
        if (readCursor[0] == '\\' && readCursor[1] == 'n')
        {
            *writeCursor++ = '\n';
            readCursor += 2;
            continue;
        }

        if (readCursor[0] == '\\' && readCursor[1] == '\\')
        {
            *writeCursor++ = '\\';
            readCursor += 2;
            continue;
        }

        *writeCursor++ = *readCursor++;
    }

    *writeCursor = '\0';
}

// Free duplicated string fields for a contiguous set of SceneLine entries.
static void FreeSceneLines(SceneLine* lines, int count)
{
    if (!lines || count <= 0) return;

    for (int i = 0; i < count; i++)
    {
        free((void*)lines[i].characterName);
        free((void*)lines[i].dialogText);
        free((void*)lines[i].backgroundPath);
        free((void*)lines[i].characterImagePath);
    }
}

// Parse one scene-script line into a SceneLine structure.
static bool ParseSceneDataLine(char* rawLine, SceneLine* outLine)
{
    char* fields[7] = {0};
    int fieldCount = 0;
    char* cursor = rawLine;
    SceneLine parsedLine = {0};

    if (!rawLine || !outLine) return false;

    while (fieldCount < 7)
    {
        char* separator = strchr(cursor, '|');
        fields[fieldCount++] = cursor;

        if (!separator) break;
        *separator = '\0';
        cursor = separator + 1;
    }

    if (fieldCount != 7) return false;

    for (int i = 0; i < 7; i++)
    {
        fields[i] = TrimWhitespace(fields[i]);
    }

    if (fields[0][0] == '\0' || fields[1][0] == '\0') return false;

    UnescapeEscapedNewlines(fields[1]);

    parsedLine.characterName = CloneString(fields[0]);
    parsedLine.dialogText = CloneString(fields[1]);
    parsedLine.characterX = strtof(fields[4], NULL);
    parsedLine.characterY = strtof(fields[5], NULL);
    parsedLine.characterScale = strtof(fields[6], NULL);

    if (!parsedLine.characterName || !parsedLine.dialogText)
    {
        FreeSceneLines(&parsedLine, 1);
        return false;
    }

    if (parsedLine.characterScale <= 0.0f) parsedLine.characterScale = 1.0f;

    if (!IsNullField(fields[2]))
    {
        parsedLine.backgroundPath = CloneString(fields[2]);
        if (!parsedLine.backgroundPath)
        {
            FreeSceneLines(&parsedLine, 1);
            return false;
        }
    }

    if (!IsNullField(fields[3]))
    {
        parsedLine.characterImagePath = CloneString(fields[3]);
        if (!parsedLine.characterImagePath)
        {
            FreeSceneLines(&parsedLine, 1);
            return false;
        }
    }

    *outLine = parsedLine;
    return true;
}
