#include "input.h"
#include "world.h"
#include "common.h"
#include "raylib.h"

static int selected = SAND;
static int brush_radius = 3;
static bool paused = false;

void init_input(void) {
    selected = SAND;
    brush_radius = 3;
    paused = false;
}

int get_selected_material(void) {
    return selected;
}

void set_selected_material(int material) {
    selected = material;
}

int get_brush_radius(void) {
    return brush_radius;
}

void set_brush_radius(int radius) {
    brush_radius = radius;
}

bool is_paused(void) {
    return paused;
}

void set_paused(bool pause) {
    paused = pause;
}

void paint_at_mouse(void) {
    Vector2 mouse = GetMousePosition();
    int cx = (int)mouse.x / CELL;
    int cy = (int)mouse.y / CELL;

    if (!inside(cx, cy)) {
        return;
    }

    for (int y = cy - brush_radius; y <= cy + brush_radius; y++) {
        for (int x = cx - brush_radius; x <= cx + brush_radius; x++) {
            if (!inside(x, y)) {
                continue;
            }

            int dx = x - cx;
            int dy = y - cy;

            if (dx * dx + dy * dy > brush_radius * brush_radius) {
                continue;
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                world[y][x] = (unsigned char)selected;
                wood_health[y][x] = 0;
                leaves_ash[y][x] = 0;

                if (selected == FIRE) {
                    fire_life[y][x] = (unsigned char)GetRandomValue(45, 95);
                    fire_falling[y][x] = (unsigned char)GetRandomValue(60, 100);
                    smoke_life[y][x] = 0;
                    temperature[y][x] = FIRE_TEMP;
                } else {
                    fire_life[y][x] = 0;
                    fire_falling[y][x] = 0;
                    if (selected == WATER) {
                        temperature[y][x] = 15.0f;
                        smoke_life[y][x] = 0;
                    } else if (selected == OIL) {
                        temperature[y][x] = 20.0f;
                        smoke_life[y][x] = 0;
                    } else if (selected == SMOKE) {
                        smoke_life[y][x] = (unsigned char)GetRandomValue(60, 110);
                        temperature[y][x] = 35.0f;
                    } else {
                        smoke_life[y][x] = 0;
                        temperature[y][x] = AMBIENT_TEMP;
                    }
                }

                grass_timer[y][x] = (selected == GRASS)
                    ? (unsigned char)GetRandomValue(45, 120) : 0;
            } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                world[y][x] = EMPTY;
                fire_life[y][x] = 0;
                fire_falling[y][x] = 0;
                grass_timer[y][x] = 0;
                smoke_life[y][x] = 0;
                wood_health[y][x] = 0;
                leaves_ash[y][x] = 0;
                temperature[y][x] = AMBIENT_TEMP;
            }
        }
    }
}

void handle_input(void) {
    if (IsKeyPressed(KEY_ONE))   selected = SAND;
    if (IsKeyPressed(KEY_TWO))   selected = WATER;
    if (IsKeyPressed(KEY_THREE)) selected = GRAVEL;
    if (IsKeyPressed(KEY_FOUR))  selected = FIRE;
    if (IsKeyPressed(KEY_FIVE))  selected = GRASS;
    if (IsKeyPressed(KEY_SIX))   selected = WOOD;
    if (IsKeyPressed(KEY_SEVEN)) selected = ASH;
    if (IsKeyPressed(KEY_EIGHT)) selected = SMOKE;
    if (IsKeyPressed(KEY_NINE))  selected = OIL;
    if (IsKeyPressed(KEY_ZERO))  selected = EMPTY;
    if (IsKeyPressed(KEY_SPACE)) paused = !paused;

    if (IsKeyPressed(KEY_R)) {
        reset_world();
    }

    if (IsKeyPressed(KEY_LEFT_BRACKET) && brush_radius > 1) {
        brush_radius--;
    }
    if (IsKeyPressed(KEY_RIGHT_BRACKET) && brush_radius < 12) {
        brush_radius++;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) ||
        IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        paint_at_mouse();
    }
}
