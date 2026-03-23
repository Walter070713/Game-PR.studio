#ifndef CAMERASET_H
#define CAMERASET_H
#include "raylib.h"
#include "Map.h"
void InitCamera(Camera2D* camera,Vector2 offset);
void FollowCameraClamped(Camera2D* camera, Vector2 followPos, GameMap map, float smoothFactor);
#endif