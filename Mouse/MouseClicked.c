#include "MouseClicked.h"

bool IsOptionClicked(const char* text, int x, int y, int fontSize, Color normalColor, Color hoverColor) 
{
    //  Measure the text so we know exactly how big the "hitbox" is
    int textWidth = MeasureText(text, fontSize);
    
    //  Create a hidden rectangle (the button's collision area)
    Rectangle bounds = { (float)x, (float)y, (float)textWidth, (float)fontSize };
    
    //  Check if the mouse cursor is inside that rectangle
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    
    //  Draw the text
    DrawText(text, x, y, fontSize, isHovered ? hoverColor : normalColor);
    
    //  Return true ONLY if the mouse is over the text AND the left button is pressed
    return isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}