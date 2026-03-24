#include "window_setting.h"
void ChangeResolution(int newWidth, int newHeight, Camera2D* camera)
{
    window_width = newWidth;
    window_height = newHeight;
    window_center = (Vector2){(float)window_width/2, (float)window_height/2};
    SetWindowSize(window_width, window_height);
    camera->offset = window_center; // Update camera offset to keep player centered
}

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