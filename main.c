#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define CELL 6
#define GRID_W 160
#define GRID_H 120
#define SCREEN_W (GRID_W * CELL)
#define SCREEN_H (GRID_H * CELL)

typedef enum {
    EMPTY = 0,
    SAND,
    WATER,
    STONE,
    FIRE,
    GRASS
} ParticleType;

static unsigned char world[GRID_H][GRID_W];
static unsigned char updated[GRID_H][GRID_W];
static unsigned char fire_life[GRID_H][GRID_W];
static unsigned char grass_timer[GRID_H][GRID_W];

static int selected = SAND;
static int brush_radius = 3;
static bool paused = false;

static bool inside(int x, int y) {
    return x >= 0 && x < GRID_W && y >= 0 && y < GRID_H;
}

static void swap_cells(int x, int y, int nx, int ny) {
    unsigned char material = world[y][x];
    world[y][x] = world[ny][nx];
    world[ny][nx] = material;

    unsigned char life = fire_life[y][x];
    fire_life[y][x] = fire_life[ny][nx];
    fire_life[ny][nx] = life;

    unsigned char timer = grass_timer[y][x];
    grass_timer[y][x] = grass_timer[ny][nx];
    grass_timer[ny][nx] = timer;

    updated[ny][nx] = 1;
}

static bool empty_at(int x, int y) {
    return inside(x, y) && world[y][x] == EMPTY;
}

static bool bury_grass(int x, int y, int nx, int ny) {
    if (!inside(nx, ny) || world[ny][nx] != GRASS) {
        return false;
    }

    world[ny][nx] = SAND;
    world[y][x] = EMPTY;

    grass_timer[ny][nx] = 0;
    fire_life[ny][nx] = 0;
    updated[ny][nx] = 1;

    return true;
}

static void try_sand(int x, int y) {
    int dir = GetRandomValue(0, 1) ? 1 : -1;

    if (empty_at(x, y + 1)) {
        swap_cells(x, y, x, y + 1);
    } else if (bury_grass(x, y, x, y + 1)) {
        return;
    } else if (empty_at(x + dir, y + 1)) {
        swap_cells(x, y, x + dir, y + 1);
    } else if (bury_grass(x, y, x + dir, y + 1)) {
        return;
    } else if (empty_at(x - dir, y + 1)) {
        swap_cells(x, y, x - dir, y + 1);
    } else if (bury_grass(x, y, x - dir, y + 1)) {
        return;
    } else if (inside(x, y + 1) && world[y + 1][x] == WATER) {
        swap_cells(x, y, x, y + 1);
    }
}

static void try_water(int x, int y) {
    int dir = GetRandomValue(0, 1) ? 1 : -1;

    if (empty_at(x, y + 1)) {
        swap_cells(x, y, x, y + 1);
        return;
    }

    if (empty_at(x + dir, y + 1)) {
        swap_cells(x, y, x + dir, y + 1);
        return;
    }

    if (empty_at(x - dir, y + 1)) {
        swap_cells(x, y, x - dir, y + 1);
        return;
    }

    int spread = GetRandomValue(1, 4);
    for (int i = 1; i <= spread; i++) {
        if (empty_at(x + dir * i, y)) {
            swap_cells(x, y, x + dir * i, y);
            return;
        }
        if (!inside(x + dir * i, y) || world[y][x + dir * i] != EMPTY) {
            break;
        }
    }
}

static void try_fire(int x, int y) {
    if (fire_life[y][x] == 0) {
        fire_life[y][x] = (unsigned char)GetRandomValue(35, 90);
    } else {
        fire_life[y][x]--;
    }

    // Fire rises into empty space.
    if (empty_at(x, y - 1) && GetRandomValue(0, 99) < 65) {
        swap_cells(x, y, x, y - 1);
        return;
    }

    // Fire can ignite nearby sand only in this simple material model.
    for (int oy = -1; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            int nx = x + ox;
            int ny = y + oy;

            if (inside(nx, ny) && world[ny][nx] == SAND &&
                GetRandomValue(0, 999) < 5) {
                world[ny][nx] = FIRE;
                fire_life[ny][nx] = (unsigned char)GetRandomValue(25, 60);
            }
        }
    }

    if (fire_life[y][x] == 0) {
        world[y][x] = EMPTY;
    }
}

static void try_grass(int x, int y) {
    if (grass_timer[y][x] > 0) {
        grass_timer[y][x]--;
        return;
    }

    int dir = GetRandomValue(0, 1) ? 1 : -1;
    int nx = x + dir;

    // Grass spreads sideways onto empty cells supported by stone.
    if (inside(nx, y) &&
        empty_at(nx, y) &&
        inside(nx, y + 1) &&
        world[y + 1][nx] == STONE) {
        world[y][nx] = GRASS;
        grass_timer[y][nx] = (unsigned char)GetRandomValue(45, 120);
    }

    // Delay the next growth attempt by this cell.
    grass_timer[y][x] = (unsigned char)GetRandomValue(45, 120);
}

static void update_world(void) {
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            updated[y][x] = 0;
        }
    }

    // Bottom-to-top update lets falling materials move naturally.
    for (int y = GRID_H - 1; y >= 0; y--) {
        int start = GetRandomValue(0, 1) ? 0 : GRID_W - 1;
        int end = (start == 0) ? GRID_W : -1;
        int step = (start == 0) ? 1 : -1;

        for (int x = start; x != end; x += step) {
            if (updated[y][x]) {
                continue;
            }

            switch (world[y][x]) {
                case SAND:  try_sand(x, y); break;
                case WATER: try_water(x, y); break;
                case FIRE:  try_fire(x, y); break;
                case GRASS: try_grass(x, y); break;
                default: break;
            }
        }
    }
}

static void paint_at_mouse(void) {
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
                fire_life[y][x] = selected == FIRE
                    ? (unsigned char)GetRandomValue(35, 90) : 0;
                grass_timer[y][x] = selected == GRASS
                    ? (unsigned char)GetRandomValue(45, 120) : 0;
            } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                world[y][x] = EMPTY;
                fire_life[y][x] = 0;
                grass_timer[y][x] = 0;
            }
        }
    }
}

static Color material_color(int material, int x, int y) {
    switch (material) {
        case SAND:
            return (GetRandomValue(0, 8) == 0)
                ? (Color){235, 196, 105, 255}
                : (Color){218, 178, 91, 255};
        case WATER:
            return (Color){45, 125 + (unsigned char)((x + y) % 35), 225, 255};
        case STONE:
            return (Color){105, 110, 120, 255};
        case FIRE: {
            int life = fire_life[y][x];
            return life > 45 ? (Color){255, 220, 65, 255}
                 : life > 20 ? (Color){255, 125, 35, 255}
                             : (Color){220, 55, 25, 255};
        }
        case GRASS:
            return (Color){65, 180, 75, 255};
        default:
            return BLANK;
    }
}

static void draw_world(void) {
    ClearBackground((Color){18, 20, 27, 255});

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            int material = world[y][x];
            if (material == EMPTY) {
                continue;
            }

            DrawRectangle(x * CELL, y * CELL, CELL, CELL,
                          material_color(material, x, y));
        }
    }
}

static const char *material_name(int material) {
    switch (material) {
        case SAND: return "SAND";
        case WATER: return "WATER";
        case STONE: return "STONE";
        case FIRE: return "FIRE";
        case GRASS: return "GRASS";
        default: return "ERASER";
    }
}

int main(void) {
    srand((unsigned int)time(NULL));
    SetRandomSeed((unsigned int)time(NULL));

    InitWindow(SCREEN_W, SCREEN_H, "C Falling Sand Simulator");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ONE)) selected = SAND;
        if (IsKeyPressed(KEY_TWO)) selected = WATER;
        if (IsKeyPressed(KEY_THREE)) selected = STONE;
        if (IsKeyPressed(KEY_FOUR)) selected = FIRE;
        if (IsKeyPressed(KEY_FIVE)) selected = GRASS;
        if (IsKeyPressed(KEY_ZERO)) selected = EMPTY;
        if (IsKeyPressed(KEY_SPACE)) paused = !paused;

        if (IsKeyPressed(KEY_R)) {
            for (int y = 0; y < GRID_H; y++) {
                for (int x = 0; x < GRID_W; x++) {
                    world[y][x] = EMPTY;
                    fire_life[y][x] = 0;
                    grass_timer[y][x] = 0;
                }
            }
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

        if (!paused) {
            update_world();
        }

        BeginDrawing();
        draw_world();

        DrawRectangle(8, 8, 430, 66, (Color){0, 0, 0, 175});
        DrawText(TextFormat("Material: %s   Brush: %d",
                            material_name(selected), brush_radius),
                 18, 16, 18, RAYWHITE);
        DrawText("1 Sand  2 Water  3 Stone  4 Fire  5 Grass  0 Eraser",
                 18, 39, 13, LIGHTGRAY);
        DrawText("SPACE Pause | R Reset | [ ] Brush | Right-click Erase",
                 18, 56, 11, LIGHTGRAY);

        if (paused) {
            DrawRectangle(SCREEN_W / 2 - 55, 12, 110, 30,
                          (Color){160, 45, 35, 220});
            DrawText("PAUSED", SCREEN_W / 2 - 39, 19, 18, RAYWHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
