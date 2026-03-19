#ifndef WINDOWSETTING_H
#define WINDOWSETTING_H
#include "raylib.h"
extern int window_width;
extern int window_height;
extern bool isFullscreen;
extern Vector2 window_center;
void ChangeResolution(int newWidth, int newHeight, Camera2D* camera);
void ToggleFullscreenMode();
#endif