#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "world.h"
#include "renderer.h"
#include "input.h"

int main(void) {
    srand((unsigned int)time(NULL));
    SetRandomSeed((unsigned int)time(NULL));

    init_world();
    init_input();

    InitWindow(SCREEN_W, SCREEN_H, "C Falling Sand Simulator");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        handle_input();

        if (!is_paused()) {
            update_world();
        }

        BeginDrawing();
        draw_world();
        draw_ui();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
