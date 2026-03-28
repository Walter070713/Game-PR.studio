#ifndef MOUSECLICKED_H
#define MOUSECLICKED_H
#include "raylib.h"

// Draw a text option and return true on hover + left-click in same frame.
bool IsOptionClicked(const char* text, int x, int y, int fontSize, Color normalColor, Color hoverColor);
#endif