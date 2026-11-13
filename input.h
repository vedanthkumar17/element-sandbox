#ifndef INPUT_H
#define INPUT_H

#include "common.h"

// Lifecycle & processing
void init_input(void);
void handle_input(void);
void paint_at_mouse(void);

// Input state accessors
int get_selected_material(void);
void set_selected_material(int material);

int get_brush_radius(void);
void set_brush_radius(int radius);

bool is_paused(void);
void set_paused(bool pause);

#endif // INPUT_H
