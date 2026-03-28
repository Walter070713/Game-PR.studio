#ifndef MOUSEAIM_H
#define MOUSEAIM_H

#include "raylib.h"

// Cached mouse world position + normalized aim direction from player.
typedef struct MseAim {
    Vector2 pos;
    Vector2 dir;
} MseAim;

// Convert cursor to world space and compute normalized aim vector.
void UpdateMouseAim(MseAim* mouse, Camera2D camera, Vector2 playerPos);

#endif