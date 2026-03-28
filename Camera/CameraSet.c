#include "CameraSet.h"

// Initialize baseline camera values used before runtime zoom/target updates.
void InitCamera(Camera2D* camera, Vector2 offset)
{
    if (!camera) return;

    camera->offset = offset;
    camera->rotation = 0.0f;
    camera->zoom = 1.0f;
}