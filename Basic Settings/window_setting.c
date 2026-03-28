#include "window_setting.h"

// Shared window state used by menus, camera setup, and save/load resume paths.
int window_width = 2560;
int window_height = 1600;
bool isFullscreen = false;
Vector2 window_center = {1280.0f, 800.0f};

// Recompute cached window midpoint after any size change.
static void UpdateWindowCenter(void)
{
    window_center = (Vector2){(float)window_width * 0.5f, (float)window_height * 0.5f};
}

// Internal helper that applies resolution state and keeps camera centered.
static void ChangeResolution(int newWidth, int newHeight, Camera2D* camera)
{
    window_width = newWidth;
    window_height = newHeight;
    UpdateWindowCenter();
    SetWindowSize(window_width, window_height);
    if (camera) camera->offset = window_center;
}

// Initialize window globals before creating the actual raylib window.
void InitWindowSettings(int initialWidth, int initialHeight)
{
    window_width = initialWidth;
    window_height = initialHeight;
    isFullscreen = false;
    UpdateWindowCenter();
}

// Apply a windowed resolution, leaving fullscreen mode first when required.
void ApplyWindowedResolution(int newWidth, int newHeight, Camera2D* camera)
{
    if (isFullscreen) ToggleFullscreenMode();
    ChangeResolution(newWidth, newHeight, camera);
}

// Toggle fullscreen and keep internal state synchronized with raylib.
void ToggleFullscreenMode() 
{
    if (!isFullscreen) 
    {
        // Going to fullscreen - use current window size
        ToggleFullscreen();
        isFullscreen = true;
    } 
    else 
    {
        // Going back to windowed
        ToggleFullscreen();
        isFullscreen = false;
        // Restore window size
        SetWindowSize(window_width, window_height);
    }

    // Safety: keep mouse cursor visible/unlocked after mode switches.
    EnableCursor();
    ShowCursor();
}