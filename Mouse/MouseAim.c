#include "MouseAim.h"
#include "raymath.h"

// Convert cursor position to world coordinates and compute aim direction.
void UpdateMouseAim(MseAim* mouse, Camera2D camera, Vector2 playerPos)
{
    if (!mouse) return;

    mouse->pos = GetMousePosition();
    mouse->pos = GetScreenToWorld2D(mouse->pos, camera);
    mouse->dir = Vector2Subtract(mouse->pos, playerPos);

    // Keep zero-length vectors from producing undefined normalization output.
    if (Vector2LengthSqr(mouse->dir) > 0.0001f)
    {
        mouse->dir = Vector2Normalize(mouse->dir);
    }
    else
    {
        mouse->dir = (Vector2){0.0f, 0.0f};
    }
}