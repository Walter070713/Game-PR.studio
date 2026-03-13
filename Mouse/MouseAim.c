#include "MouseAim.h"
// Logic to make player keep aiming at where the cursor at
void UpdateMouseAim(MseAim* mouse,Camera2D camera,Vector2 plpos)
{
    mouse->pos=GetMousePosition();
    mouse->pos=GetScreenToWorld2D(mouse->pos,camera);
    mouse->dir=Vector2Subtract(mouse->pos,plpos);
    mouse->dir=Vector2Normalize(mouse->dir);
}