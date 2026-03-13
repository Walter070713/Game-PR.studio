#include "CameraSet.h"
// Initialize the camera setting
void InitCamera(Camera2D* camera,Vector2 offset)
{
    camera->offset=offset;
    camera->rotation=0.0f;
    camera->zoom=1.0f;
}