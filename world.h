#ifndef WORLD_H
#define WORLD_H

#include "common.h"

// Global simulation grid state
extern unsigned char world[GRID_H][GRID_W];
extern unsigned char updated[GRID_H][GRID_W];
extern unsigned char fire_life[GRID_H][GRID_W];
extern unsigned char fire_falling[GRID_H][GRID_W];
extern unsigned char grass_timer[GRID_H][GRID_W];
extern float temperature[GRID_H][GRID_W];

// Lifecycle & simulation functions
void init_world(void);
void reset_world(void);
void update_world(void);
void update_temperature(void);

// Grid inspection & manipulation utilities
bool inside(int x, int y);
bool empty_at(int x, int y);
void swap_cells(int x, int y, int nx, int ny);

#endif // WORLD_H
