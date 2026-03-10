#include "MouseAim.h"
void UpdateMouseAim(MseAim* mouse,Camera2D camera,Player* pl)
{
    mouse->pos=GetMousePosition();
    mouse->pos=GetScreenToWorld2D(mouse->pos,camera);
    mouse->dir=Vector2Subtract(mouse->pos,pl->pos);
    mouse->dir=Vector2Normalize(mouse->dir);
}