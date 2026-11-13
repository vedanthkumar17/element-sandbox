#ifndef PARTICLES_H
#define PARTICLES_H

#include "common.h"

// Material-specific simulation logic
void try_sand(int x, int y);
void try_water(int x, int y);
void try_gravel(int x, int y);
void try_fire(int x, int y);
void try_grass(int x, int y);
void try_wood(int x, int y);
void try_ash(int x, int y);
void try_smoke(int x, int y);
void try_oil(int x, int y);

#endif // PARTICLES_H
