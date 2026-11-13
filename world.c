#include "world.h"
#include "particles.h"
#include "common.h"
#include "raylib.h"

// Simulation grid storage
unsigned char world[GRID_H][GRID_W];
unsigned char updated[GRID_H][GRID_W];
unsigned char fire_life[GRID_H][GRID_W];
unsigned char fire_falling[GRID_H][GRID_W];
unsigned char grass_timer[GRID_H][GRID_W];
unsigned char smoke_life[GRID_H][GRID_W];
unsigned char wood_health[GRID_H][GRID_W];
unsigned char leaves_ash[GRID_H][GRID_W];
float temperature[GRID_H][GRID_W];

bool inside(int x, int y) {
    return x >= 0 && x < GRID_W && y >= 0 && y < GRID_H;
}

bool empty_at(int x, int y) {
    return inside(x, y) && world[y][x] == EMPTY;
}

void swap_cells(int x, int y, int nx, int ny) {
    unsigned char material = world[y][x];
    world[y][x] = world[ny][nx];
    world[ny][nx] = material;

    unsigned char life = fire_life[y][x];
    fire_life[y][x] = fire_life[ny][nx];
    fire_life[ny][nx] = life;

    unsigned char falling = fire_falling[y][x];
    fire_falling[y][x] = fire_falling[ny][nx];
    fire_falling[ny][nx] = falling;

    unsigned char timer = grass_timer[y][x];
    grass_timer[y][x] = grass_timer[ny][nx];
    grass_timer[ny][nx] = timer;

    unsigned char slife = smoke_life[y][x];
    smoke_life[y][x] = smoke_life[ny][nx];
    smoke_life[ny][nx] = slife;

    unsigned char whealth = wood_health[y][x];
    wood_health[y][x] = wood_health[ny][nx];
    wood_health[ny][nx] = whealth;

    unsigned char lash = leaves_ash[y][x];
    leaves_ash[y][x] = leaves_ash[ny][nx];
    leaves_ash[ny][nx] = lash;

    float t = temperature[y][x];
    temperature[y][x] = temperature[ny][nx];
    temperature[ny][nx] = t;

    updated[ny][nx] = 1;
}

void update_temperature(void) {
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (temperature[y][x] > AMBIENT_TEMP) {
                float diff = temperature[y][x] - AMBIENT_TEMP;
                temperature[y][x] -= diff * 0.04f;
                if (temperature[y][x] < AMBIENT_TEMP + 0.5f) {
                    temperature[y][x] = AMBIENT_TEMP;
                }
            } else if (temperature[y][x] < AMBIENT_TEMP) {
                temperature[y][x] += (AMBIENT_TEMP - temperature[y][x]) * 0.04f;
            }
        }
    }
}

void update_world(void) {
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
                case GRAVEL: try_gravel(x, y); break;
                case FIRE:  try_fire(x, y); break;
                case GRASS: try_grass(x, y); break;
                case WOOD:  try_wood(x, y); break;
                case ASH:   try_ash(x, y); break;
                case SMOKE: try_smoke(x, y); break;
                case OIL:   try_oil(x, y); break;
                default: break;
            }
        }
    }

    update_temperature();
}

void reset_world(void) {
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            world[y][x] = EMPTY;
            updated[y][x] = 0;
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

void init_world(void) {
    reset_world();
}
