#include "CameraSet.h"
// Initialize the camera setting
void InitCamera(Camera2D* camera,Vector2 offset)
{
    camera->offset=offset;
    camera->rotation=0.0f;
    camera->zoom=1.0f;
}

void FollowCameraClamped(Camera2D* camera, Vector2 followPos, GameMap map, float smoothFactor)
{
    if (!camera) return;

    float t = smoothFactor;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    // First follow the target position smoothly.
    camera->target.x = followPos.x + (camera->target.x - followPos.x) * t;
    camera->target.y = followPos.y + (camera->target.y - followPos.y) * t;

    float zoom = (camera->zoom <= 0.0f) ? 1.0f : camera->zoom;
    float leftView = camera->offset.x / zoom;
    float rightView = ((float)GetScreenWidth() - camera->offset.x) / zoom;
    float topView = camera->offset.y / zoom;
    float bottomView = ((float)GetScreenHeight() - camera->offset.y) / zoom;

    float minX = map.bounds.x + leftView;
    float maxX = map.bounds.x + map.bounds.width - rightView;
    float minY = map.bounds.y + topView;
    float maxY = map.bounds.y + map.bounds.height - bottomView;

    // If the room is smaller than the viewport in an axis, center camera on room.
    if (minX > maxX) {
        camera->target.x = map.bounds.x + map.bounds.width * 0.5f;
    } else {
        if (camera->target.x < minX) camera->target.x = minX;
        if (camera->target.x > maxX) camera->target.x = maxX;
    }

    if (minY > maxY) {
        camera->target.y = map.bounds.y + map.bounds.height * 0.5f;
    } else {
        if (camera->target.y < minY) camera->target.y = minY;
        if (camera->target.y > maxY) camera->target.y = maxY;
    }
}