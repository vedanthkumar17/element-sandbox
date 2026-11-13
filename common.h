#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include <stdbool.h>

// Grid and display dimensions
#define CELL 6
#define GRID_W 160
#define GRID_H 120
#define SCREEN_W (GRID_W * CELL)
#define SCREEN_H (GRID_H * CELL)

// Thermal simulation constants
#define AMBIENT_TEMP 20.0f
#define FIRE_TEMP 450.0f
#define GRASS_IGNITE_TEMP 100.0f

// Particle material identifiers
typedef enum {
    EMPTY = 0,
    SAND,
    WATER,
    STONE,
    FIRE,
    GRASS
} ParticleType;

#endif // COMMON_H
