#ifndef PARTICLES_H
#define PARTICLES_H

#include "common.h"

// Material-specific simulation logic
void try_sand(int x, int y);
void try_water(int x, int y);
void try_fire(int x, int y);
void try_grass(int x, int y);

#endif // PARTICLES_H
