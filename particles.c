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
    } else if (inside(x, y + 1) && (world[y + 1][x] == WATER || world[y + 1][x] == OIL || world[y + 1][x] == ASH)) {
        // Sand displaces lighter liquids and fine ash
        swap_cells(x, y, x, y + 1);
    }
}

void try_water(int x, int y) {
    int dir = GetRandomValue(0, 1) ? 1 : -1;

    // Water is denser than oil: sink through oil directly below
    if (inside(x, y + 1) && world[y + 1][x] == OIL) {
        swap_cells(x, y, x, y + 1);
        return;
    }

    if (empty_at(x, y + 1)) {
        swap_cells(x, y, x, y + 1);
        return;
    }

    // Diagonal sink through oil or fall into empty space
    if (inside(x + dir, y + 1) && world[y + 1][x + dir] == OIL) {
        swap_cells(x, y, x + dir, y + 1);
        return;
    }
    if (empty_at(x + dir, y + 1)) {
        swap_cells(x, y, x + dir, y + 1);
        return;
    }

    if (inside(x - dir, y + 1) && world[y + 1][x - dir] == OIL) {
        swap_cells(x, y, x - dir, y + 1);
        return;
    }
    if (empty_at(x - dir, y + 1)) {
        swap_cells(x, y, x - dir, y + 1);
        return;
    }

    // Spread sideways on flat surfaces
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

void try_stone(int x, int y) {
    int dir = GetRandomValue(0, 1) ? 1 : -1;

    // Stone falls and piles up like gravel/sand
    if (empty_at(x, y + 1)) {
        swap_cells(x, y, x, y + 1);
    } else if (empty_at(x + dir, y + 1)) {
        swap_cells(x, y, x + dir, y + 1);
    } else if (empty_at(x - dir, y + 1)) {
        swap_cells(x, y, x - dir, y + 1);
    } else if (inside(x, y + 1) && (world[y + 1][x] == WATER || world[y + 1][x] == OIL || world[y + 1][x] == ASH)) {
        // Heavy stone sinks through liquids and light ash
        swap_cells(x, y, x, y + 1);
    }
}

void try_fire(int x, int y) {
    // Check for water interaction:
    // Oil fire floating on top of water (water only underneath) continues burning;
    // fire submerged or flanked by water is extinguished.
    bool water_above_or_sides = false;
    bool water_below = false;
    for (int oy = -1; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            if (ox == 0 && oy == 0) continue;
            int nx = x + ox;
            int ny = y + oy;
            if (inside(nx, ny) && world[ny][nx] == WATER) {
                if (oy <= 0) water_above_or_sides = true;
                else water_below = true;
            }
        }
    }

    if (water_above_or_sides || (water_below && fire_life[y][x] <= 70)) {
        world[y][x] = EMPTY;
        fire_life[y][x] = 0;
        fire_falling[y][x] = 0;
        leaves_ash[y][x] = 0;
        temperature[y][x] = 30.0f;
        return;
    }

    // Maintain high temperature and generate upward thermal plume
    temperature[y][x] = FIRE_TEMP;
    for (int oy = -2; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            int nx = x + ox;
            int ny = y + oy;
            if (inside(nx, ny)) {
                float target = (oy < 0) ? 320.0f : ((ox == 0 && oy == 0) ? FIRE_TEMP : 240.0f);
                if (temperature[ny][nx] < target) {
                    temperature[ny][nx] += (target - temperature[ny][nx]) * 0.35f;
                }
            }
        }
    }

    // 1. Initial falling ember phase
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
        if (leaves_ash[y][x]) {
            world[y][x] = ASH;
            leaves_ash[y][x] = 0;
            temperature[y][x] = 160.0f;
        } else {
            world[y][x] = EMPTY;
        }
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

    // Fire occasionally produces rising smoke
    if (GetRandomValue(0, 99) < 18) {
        int sdir = GetRandomValue(0, 1) ? 1 : -1;
        if (empty_at(x, y - 1)) {
            world[y - 1][x] = SMOKE;
            smoke_life[y - 1][x] = (unsigned char)GetRandomValue(45, 90);
            temperature[y - 1][x] = 180.0f;
            updated[y - 1][x] = 1;
        } else if (empty_at(x + sdir, y - 1)) {
            world[y - 1][x + sdir] = SMOKE;
            smoke_life[y - 1][x + sdir] = (unsigned char)GetRandomValue(45, 90);
            temperature[y - 1][x + sdir] = 180.0f;
            updated[y - 1][x + sdir] = 1;
        }
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
                    // Burning grass creates Fire + Smoke + Ash
                    world[ny][nx] = FIRE;
                    fire_life[ny][nx] = (unsigned char)GetRandomValue(40, 85);
                    fire_falling[ny][nx] = 0;
                    grass_timer[ny][nx] = 0;
                    leaves_ash[ny][nx] = 1;
                    temperature[ny][nx] = FIRE_TEMP;
                    updated[ny][nx] = 1;

                    if (empty_at(nx, ny - 1)) {
                        world[ny - 1][nx] = SMOKE;
                        smoke_life[ny - 1][nx] = (unsigned char)GetRandomValue(45, 85);
                        temperature[ny - 1][nx] = 150.0f;
                        updated[ny - 1][nx] = 1;
                    }
                } else if (world[ny][nx] == WOOD) {
                    // Fire ignites wood
                    if (wood_health[ny][nx] == 0) {
                        wood_health[ny][nx] = (unsigned char)GetRandomValue(90, 160);
                    }
                } else if (world[ny][nx] == OIL) {
                    // Oil catches fire easily and burns longer with thick smoke
                    world[ny][nx] = FIRE;
                    fire_life[ny][nx] = (unsigned char)GetRandomValue(140, 240);
                    fire_falling[ny][nx] = 0;
                    temperature[ny][nx] = 520.0f;
                    updated[ny][nx] = 1;

                    if (empty_at(nx, ny - 1)) {
                        world[ny - 1][nx] = SMOKE;
                        smoke_life[ny - 1][nx] = (unsigned char)GetRandomValue(70, 120);
                        temperature[ny - 1][nx] = 200.0f;
                        updated[ny - 1][nx] = 1;
                    }
                }
            }
        }
    }
}

void try_grass(int x, int y) {
    // 1. Burn when touching fire, burning wood, or when temperature reaches ignition threshold
    bool touching_fire = false;
    for (int oy = -1; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            int nx = x + ox;
            int ny = y + oy;
            if (inside(nx, ny)) {
                if (world[ny][nx] == FIRE || (world[ny][nx] == WOOD && wood_health[ny][nx] > 0)) {
                    touching_fire = true;
                    break;
                }
            }
        }
        if (touching_fire) break;
    }

    if (touching_fire || temperature[y][x] >= GRASS_IGNITE_TEMP) {
        // Burning Grass creates Ash + Smoke + Fire:
        // Leaves Ash on the ground, leaps flames upward, and emits smoke into the air
        if (empty_at(x, y - 1)) {
            world[y][x] = ASH;
            temperature[y][x] = 180.0f;
            updated[y][x] = 1;

            world[y - 1][x] = FIRE;
            fire_life[y - 1][x] = (unsigned char)GetRandomValue(40, 85);
            fire_falling[y - 1][x] = 0;
            temperature[y - 1][x] = FIRE_TEMP;
            updated[y - 1][x] = 1;

            if (empty_at(x, y - 2)) {
                world[y - 2][x] = SMOKE;
                smoke_life[y - 2][x] = (unsigned char)GetRandomValue(50, 95);
                temperature[y - 2][x] = 140.0f;
                updated[y - 2][x] = 1;
            }
        } else {
            world[y][x] = FIRE;
            fire_life[y][x] = (unsigned char)GetRandomValue(40, 85);
            fire_falling[y][x] = 0;
            leaves_ash[y][x] = 1;
            temperature[y][x] = FIRE_TEMP;
            updated[y][x] = 1;
        }

        grass_timer[y][x] = 0;
        return;
    }

    // 2. Light powder falling behavior (settles onto stone or solid surfaces)
    bool on_solid = (inside(x, y + 1) && (world[y + 1][x] == STONE || world[y + 1][x] == SAND || world[y + 1][x] == WOOD)) ||
                    (y + 1 >= GRID_H);

    if (!on_solid) {
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
            return;
        }
    }

    // 3. Spreading behavior across stone surfaces
    if (grass_timer[y][x] > 0) {
        grass_timer[y][x]--;
        return;
    }

    int dir = GetRandomValue(0, 1) ? 1 : -1;
    int nx = x + dir;

    if (inside(nx, y) &&
        empty_at(nx, y) &&
        inside(nx, y + 1) &&
        (world[y + 1][nx] == STONE || world[y + 1][nx] == WOOD)) {
        world[y][nx] = GRASS;
        grass_timer[y][nx] = (unsigned char)GetRandomValue(45, 120);
    }

    grass_timer[y][x] = (unsigned char)GetRandomValue(45, 120);
}

void try_wood(int x, int y) {
    // Check if extinguished by water
    for (int oy = -1; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            int nx = x + ox;
            int ny = y + oy;
            if (inside(nx, ny) && world[ny][nx] == WATER) {
                if (wood_health[y][x] > 0) {
                    wood_health[y][x] = 0;
                    temperature[y][x] = 30.0f;
                    return;
                }
            }
        }
    }

    // Check ignition from neighbor fire, burning wood, or high temperature
    if (wood_health[y][x] == 0) {
        bool ignited = (temperature[y][x] >= WOOD_IGNITE_TEMP);
        if (!ignited) {
            for (int oy = -1; oy <= 1; oy++) {
                for (int ox = -1; ox <= 1; ox++) {
                    int nx = x + ox;
                    int ny = y + oy;
                    if (inside(nx, ny)) {
                        if (world[ny][nx] == FIRE || (world[ny][nx] == WOOD && wood_health[ny][nx] > 0)) {
                            ignited = true;
                            break;
                        }
                    }
                }
                if (ignited) break;
            }
        }

        if (ignited) {
            wood_health[y][x] = (unsigned char)GetRandomValue(90, 160);
        }
    }

    // Burning Wood simulation
    if (wood_health[y][x] > 0) {
        wood_health[y][x]--;
        temperature[y][x] = FIRE_TEMP;

        // Radiate heat upward and outward
        for (int oy = -2; oy <= 1; oy++) {
            for (int ox = -1; ox <= 1; ox++) {
                int nx = x + ox;
                int ny = y + oy;
                if (inside(nx, ny)) {
                    float target = (oy < 0) ? 320.0f : 230.0f;
                    if (temperature[ny][nx] < target) {
                        temperature[ny][nx] += (target - temperature[ny][nx]) * 0.3f;
                    }
                }
            }
        }

        // Ignite adjacent flammables
        for (int oy = -1; oy <= 1; oy++) {
            for (int ox = -1; ox <= 1; ox++) {
                int nx = x + ox;
                int ny = y + oy;
                if (inside(nx, ny)) {
                    if (world[ny][nx] == OIL) {
                        world[ny][nx] = FIRE;
                        fire_life[ny][nx] = (unsigned char)GetRandomValue(140, 240);
                        temperature[ny][nx] = 520.0f;
                        updated[ny][nx] = 1;
                    } else if (world[ny][nx] == GRASS) {
                        world[ny][nx] = FIRE;
                        fire_life[ny][nx] = (unsigned char)GetRandomValue(40, 85);
                        temperature[ny][nx] = FIRE_TEMP;
                        updated[ny][nx] = 1;
                    } else if (world[ny][nx] == WOOD && wood_health[ny][nx] == 0) {
                        if (GetRandomValue(0, 99) < 18) {
                            wood_health[ny][nx] = (unsigned char)GetRandomValue(90, 160);
                        }
                    }
                }
            }
        }

        int dir = GetRandomValue(0, 1) ? 1 : -1;

        // Burning Wood creates Fire above/beside it
        if (GetRandomValue(0, 99) < 25) {
            if (empty_at(x, y - 1)) {
                world[y - 1][x] = FIRE;
                fire_life[y - 1][x] = (unsigned char)GetRandomValue(30, 60);
                temperature[y - 1][x] = FIRE_TEMP;
                updated[y - 1][x] = 1;
            } else if (empty_at(x + dir, y)) {
                world[y][x + dir] = FIRE;
                fire_life[y][x + dir] = (unsigned char)GetRandomValue(25, 50);
                temperature[y][x + dir] = FIRE_TEMP;
                updated[y][x + dir] = 1;
            }
        }

        // Burning Wood creates Smoke billowing upwards
        if (GetRandomValue(0, 99) < 30) {
            int sy = (inside(x, y - 1) && world[y - 1][x] != EMPTY) ? y - 2 : y - 1;
            if (empty_at(x, sy)) {
                world[sy][x] = SMOKE;
                smoke_life[sy][x] = (unsigned char)GetRandomValue(50, 100);
                temperature[sy][x] = 160.0f;
                updated[sy][x] = 1;
            } else if (empty_at(x + dir, sy)) {
                world[sy][x + dir] = SMOKE;
                smoke_life[sy][x + dir] = (unsigned char)GetRandomValue(50, 100);
                temperature[sy][x + dir] = 160.0f;
                updated[sy][x + dir] = 1;
            }
        }

        // Wood completely burned down turns into Ash!
        if (wood_health[y][x] == 0) {
            world[y][x] = ASH;
            temperature[y][x] = 220.0f;
            updated[y][x] = 1;

            if (empty_at(x, y - 1)) {
                world[y - 1][x] = SMOKE;
                smoke_life[y - 1][x] = (unsigned char)GetRandomValue(60, 110);
                updated[y - 1][x] = 1;
            }
        }
    }
}

void try_ash(int x, int y) {
    int dir = GetRandomValue(0, 1) ? 1 : -1;

    // Ash falls like sand
    if (empty_at(x, y + 1)) {
        swap_cells(x, y, x, y + 1);
    } else if (empty_at(x + dir, y + 1)) {
        swap_cells(x, y, x + dir, y + 1);
    } else if (empty_at(x - dir, y + 1)) {
        swap_cells(x, y, x - dir, y + 1);
    } else if (inside(x, y + 1) && world[y + 1][x] == WATER && GetRandomValue(0, 99) < 40) {
        // Ash slowly settles into water
        swap_cells(x, y, x, y + 1);
    }
}

void try_smoke(int x, int y) {
    if (smoke_life[y][x] == 0) {
        world[y][x] = EMPTY;
        return;
    }
    smoke_life[y][x]--;

    if (smoke_life[y][x] == 0) {
        world[y][x] = EMPTY;
        return;
    }

    // Smoke rises through liquids
    if (inside(x, y - 1) && (world[y - 1][x] == WATER || world[y - 1][x] == OIL)) {
        swap_cells(x, y, x, y - 1);
        return;
    }

    int dir = GetRandomValue(0, 1) ? 1 : -1;

    // Smoke rises upward and dissipates
    if (empty_at(x, y - 1) && GetRandomValue(0, 99) < 85) {
        swap_cells(x, y, x, y - 1);
        return;
    } else if (empty_at(x + dir, y - 1) && GetRandomValue(0, 99) < 60) {
        swap_cells(x, y, x + dir, y - 1);
        return;
    } else if (empty_at(x - dir, y - 1) && GetRandomValue(0, 99) < 60) {
        swap_cells(x, y, x - dir, y - 1);
        return;
    } else if (empty_at(x + dir, y) && GetRandomValue(0, 99) < 40) {
        // Disperse sideways
        swap_cells(x, y, x + dir, y);
        return;
    }
}

void try_oil(int x, int y) {
    // 1. High flammability: catches fire easily from fire, burning wood, or heat
    bool touching_fire = false;
    for (int oy = -1; oy <= 1; oy++) {
        for (int ox = -1; ox <= 1; ox++) {
            int nx = x + ox;
            int ny = y + oy;
            if (inside(nx, ny)) {
                if (world[ny][nx] == FIRE || (world[ny][nx] == WOOD && wood_health[ny][nx] > 0)) {
                    touching_fire = true;
                    break;
                }
            }
        }
        if (touching_fire) break;
    }

    if (touching_fire || temperature[y][x] >= OIL_IGNITE_TEMP) {
        // Oil bursts into long-lasting, hot flames with thick smoke
        world[y][x] = FIRE;
        fire_life[y][x] = (unsigned char)GetRandomValue(140, 240);
        fire_falling[y][x] = 0;
        temperature[y][x] = 520.0f;
        updated[y][x] = 1;

        // Produce thick dark smoke
        if (empty_at(x, y - 1)) {
            world[y - 1][x] = SMOKE;
            smoke_life[y - 1][x] = (unsigned char)GetRandomValue(70, 130);
            temperature[y - 1][x] = 220.0f;
            updated[y - 1][x] = 1;
        }

        // Rapidly ignite neighboring oil cells
        for (int oy = -1; oy <= 1; oy++) {
            for (int ox = -1; ox <= 1; ox++) {
                int nx = x + ox;
                int ny = y + oy;
                if (inside(nx, ny) && world[ny][nx] == OIL) {
                    world[ny][nx] = FIRE;
                    fire_life[ny][nx] = (unsigned char)GetRandomValue(140, 240);
                    temperature[ny][nx] = 520.0f;
                    updated[ny][nx] = 1;
                }
            }
        }
        return;
    }

    // 2. Floats on water: if submerged under water, float upward
    if (inside(x, y - 1) && world[y - 1][x] == WATER) {
        swap_cells(x, y, x, y - 1);
        return;
    }

    // 3. Falling & spreading
    int dir = GetRandomValue(0, 1) ? 1 : -1;

    // Fall downward into empty space
    if (empty_at(x, y + 1)) {
        swap_cells(x, y, x, y + 1);
        return;
    }

    // Fall diagonally into empty space
    if (empty_at(x + dir, y + 1)) {
        swap_cells(x, y, x + dir, y + 1);
        return;
    }
    if (empty_at(x - dir, y + 1)) {
        swap_cells(x, y, x - dir, y + 1);
        return;
    }

    // Spread sideways on surfaces or across the top of water
    int spread = GetRandomValue(1, 3);
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
