#ifndef MOUSEAIM_h
#define MOUSEAIM_H
#include "raylib.h"
#include "raymath.h"
#include "Player.h"
typedef struct MseAim{
    Vector2 pos;
    Vector2 dir;
}MseAim;
void UpdateMouseAim(MseAim* mouse,Camera2D camera,Player* pl);
#endif