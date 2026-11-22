#ifndef RENDERER_H
#define RENDERER_H

#include "common.h"
#include "raylib.h"

// World and UI rendering
void draw_world(void);
void draw_ui(void);

// Material visual properties
Color material_color(int material, int x, int y);
const char *material_name(int material);
Color material_ui_color(int material);

#endif // RENDERER_H
