#include "particles.h"
#include "world.h"
#include "common.h"
#include "raylib.h"

// Helper: sand falling onto grass buries it and turns the cell into sand
static bool bury_grass(int x, int y, int nx, int ny) {
    if (!inside(nx, ny) || world[ny][nx] != GRASS) {
        return false;
    }

    world[ny][nx] = SAND;
    world[y][x] = EMPTY;

    grass_timer[ny][nx] = 0;
    fire_life[ny][nx] = 0;
    fire_falling[ny][nx] = 0;
    updated[ny][nx] = 1;

    return true;
}

void try_sand(int x, int y) {
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

void try_water(int x, int y) {
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

void try_fire(int x, int y) {
    // Extinguish if touching water
    for (int oy = -1; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            int nx = x + ox;
            int ny = y + oy;
            if (inside(nx, ny) && world[ny][nx] == WATER) {
                world[y][x] = EMPTY;
                fire_life[y][x] = 0;
                fire_falling[y][x] = 0;
                temperature[y][x] = 30.0f;
                return;
            }
        }
    }

    // Maintain high temperature and heat nearby cells
    temperature[y][x] = FIRE_TEMP;
    for (int oy = -2; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            int nx = x + ox;
            int ny = y + oy;
            if (inside(nx, ny)) {
                // Thermal plume: heat rises strongly upward
                float target = (oy < 0) ? 320.0f : ((ox == 0 && oy == 0) ? FIRE_TEMP : 240.0f);
                if (temperature[ny][nx] < target) {
                    temperature[ny][nx] += (target - temperature[ny][nx]) * 0.35f;
                }
            }
        }
    }

    // 1. Initial falling phase: falls downward like a light particle
    if (fire_falling[y][x] > 0) {
        fire_falling[y][x]--;

        int dir = GetRandomValue(0, 1) ? 1 : -1;
        if (empty_at(x, y + 1)) {
            if (GetRandomValue(0, 99) < 75) {
                swap_cells(x, y, x, y + 1);
            }
            return;
        } else if (empty_at(x + dir, y + 1)) {
            if (GetRandomValue(0, 99) < 65) {
                swap_cells(x, y, x + dir, y + 1);
            }
            return;
        } else if (empty_at(x - dir, y + 1)) {
            if (GetRandomValue(0, 99) < 65) {
                swap_cells(x, y, x - dir, y + 1);
            }
            return;
        } else {
            // Landed on a surface/particle: transition to rising
            fire_falling[y][x] = 0;
        }
    }

    // 2. Rising phase & lifetime countdown
    if (fire_life[y][x] == 0) {
        fire_life[y][x] = (unsigned char)GetRandomValue(35, 90);
    } else {
        fire_life[y][x]--;
    }

    if (fire_life[y][x] == 0) {
        world[y][x] = EMPTY;
        return;
    }

    // Fire rises upward into empty space
    int rdir = GetRandomValue(0, 1) ? 1 : -1;
    if (empty_at(x, y - 1) && GetRandomValue(0, 99) < 65) {
        swap_cells(x, y, x, y - 1);
        return;
    } else if (empty_at(x + rdir, y - 1) && GetRandomValue(0, 99) < 30) {
        swap_cells(x, y, x + rdir, y - 1);
        return;
    }

    // Direct ignition of adjacent flammable materials
    for (int oy = -1; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            int nx = x + ox;
            int ny = y + oy;

            if (inside(nx, ny)) {
                if (world[ny][nx] == SAND && GetRandomValue(0, 999) < 5) {
                    world[ny][nx] = FIRE;
                    fire_life[ny][nx] = (unsigned char)GetRandomValue(25, 60);
                    fire_falling[ny][nx] = 0;
                    temperature[ny][nx] = FIRE_TEMP;
                } else if (world[ny][nx] == GRASS) {
                    world[ny][nx] = FIRE;
                    fire_life[ny][nx] = (unsigned char)GetRandomValue(40, 85);
                    fire_falling[ny][nx] = 0;
                    grass_timer[ny][nx] = 0;
                    temperature[ny][nx] = FIRE_TEMP;
                    updated[ny][nx] = 1;
                }
            }
        }
    }
}

void try_grass(int x, int y) {
    // 1. Burn when touching fire or when temperature gets hot enough
    bool touching_fire = false;
    for (int oy = -1; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            int nx = x + ox;
            int ny = y + oy;
            if (inside(nx, ny) && world[ny][nx] == FIRE) {
                touching_fire = true;
                break;
            }
        }
        if (touching_fire) break;
    }

    if (touching_fire || temperature[y][x] >= GRASS_IGNITE_TEMP) {
        world[y][x] = FIRE;
        fire_life[y][x] = (unsigned char)GetRandomValue(40, 85);
        fire_falling[y][x] = 0;
        grass_timer[y][x] = 0;
        temperature[y][x] = FIRE_TEMP;
        updated[y][x] = 1;
        return;
    }

    // 2. Light powder falling behavior (similar to sand, but slower/lighter feel).
    // Once it lands on stone or another solid, it stops falling and can spread.
    bool on_solid = (inside(x, y + 1) && (world[y + 1][x] == STONE || world[y + 1][x] == SAND)) ||
                    (y + 1 >= GRID_H);

    if (!on_solid) {
        // Falls on ~65% of frames for a lighter feel than sand
        if (GetRandomValue(0, 99) < 65) {
            int dir = GetRandomValue(0, 1) ? 1 : -1;

            if (empty_at(x, y + 1)) {
                swap_cells(x, y, x, y + 1);
                return;
            } else if (empty_at(x + dir, y + 1)) {
                swap_cells(x, y, x + dir, y + 1);
                return;
            } else if (empty_at(x - dir, y + 1)) {
                swap_cells(x, y, x - dir, y + 1);
                return;
            }
        } else if (empty_at(x, y + 1) || empty_at(x - 1, y + 1) || empty_at(x + 1, y + 1)) {
            // Still drifting in the air; do not spread while falling
            return;
        }
    }

    // 3. Spreading behavior (sideways spreading logic onto empty cells above stone)
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
