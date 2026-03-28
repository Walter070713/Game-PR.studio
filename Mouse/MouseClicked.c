#include "MouseClicked.h"

// Draw one text option and treat its text bounds as click target.
bool IsOptionClicked(const char* text, int x, int y, int fontSize, Color normalColor, Color hoverColor) 
{
    // Compute hitbox from measured text width.
    int textWidth = MeasureText(text, fontSize);
    
    // Use text rectangle as hover/click region.
    Rectangle bounds = { (float)x, (float)y, (float)textWidth, (float)fontSize };
    
    // Highlight when cursor is inside region.
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    
    // Render option label in normal/hover color.
    DrawText(text, x, y, fontSize, isHovered ? hoverColor : normalColor);
    
    // Trigger on same-frame left click while hovered.
    return isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}