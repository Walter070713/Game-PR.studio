#ifndef WINDOWSETTING_H
#define WINDOWSETTING_H
#include "raylib.h"

// Global window/runtime display state shared by UI and camera logic.

extern int window_width;
extern int window_height;
extern bool isFullscreen;
extern Vector2 window_center;

// Initialize the shared window globals before InitWindow().
void InitWindowSettings(int initialWidth, int initialHeight);

// Force windowed mode and apply a new window resolution.
void ApplyWindowedResolution(int newWidth, int newHeight, Camera2D* camera);

// Toggle fullscreen/windowed state while preserving shared globals.
void ToggleFullscreenMode();

#endif