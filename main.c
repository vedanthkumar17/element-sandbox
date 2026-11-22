#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "world.h"
#include "renderer.h"
#include "input.h"

int main(int argc, char **argv) {
    bool test_mode = (argc > 1 && strcmp(argv[1], "--test") == 0);

    srand((unsigned int)time(NULL));
    SetRandomSeed((unsigned int)time(NULL));

    init_world();
    init_input();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_W, SCREEN_H, "Element Sandbox");
    SetWindowMinSize(740, 480);
    SetTargetFPS(60);

    int test_frame = 0;
    int test_status = 0;

    while (!WindowShouldClose()) {
        handle_input();

        if (test_mode) {
            test_frame++;
            if (test_frame == 5) {
                SetWindowSize(1100, 750);
            }
            if (test_frame == 10) {
                int w = GetScreenWidth();
                int h = GetScreenHeight();
                printf("[TEST] Resized window dimensions: %d x %d\n", w, h);
                toggle_fullscreen();
            }
            if (test_frame == 12) {
                // Test mouse painting in fullscreen
                SetMousePosition(768, 432);
                set_selected_material(SAND);
                set_brush_radius(2);
                // Simulate left mouse button by setting cell directly via paint_at_mouse logic
                int cx = 768 * GRID_W / GetScreenWidth();
                int cy = 432 * GRID_H / GetScreenHeight();
                if (cx == 80 && cy == 60) {
                    printf("[TEST] Mouse coordinate mapping in fullscreen: (768, 432) -> grid (%d, %d) [CORRECT]\n", cx, cy);
                } else {
                    fprintf(stderr, "[TEST ERROR] Mouse mapping failed! Got (%d, %d)\n", cx, cy);
                    test_status = 1;
                }
            }
            if (test_frame == 15) {
                int w = GetScreenWidth();
                int h = GetScreenHeight();
                int fs = IsWindowFullscreen();
                printf("[TEST] Fullscreen state: is_fullscreen=%d, dimensions: %d x %d\n", fs, w, h);
                if (!fs) {
                    fprintf(stderr, "[TEST ERROR] Window is not fullscreen!\n");
                    test_status = 1;
                }
            }
            if (test_frame == 20) {
                toggle_fullscreen();
            }
            if (test_frame == 25) {
                int w = GetScreenWidth();
                int h = GetScreenHeight();
                int fs = IsWindowFullscreen();
                printf("[TEST] Restored state: is_fullscreen=%d, dimensions: %d x %d\n", fs, w, h);
                if (fs) {
                    fprintf(stderr, "[TEST ERROR] Window did not exit fullscreen!\n");
                    test_status = 1;
                }
                if (w != 1100 || h != 750) {
                    fprintf(stderr, "[TEST ERROR] Dimensions not restored! Expected 1100x750, got %dx%d\n", w, h);
                    test_status = 1;
                }
                break;
            }
        }

        if (!is_paused()) {
            update_world();
        }

        BeginDrawing();
        draw_world();
        draw_ui();
        EndDrawing();
    }

    CloseWindow();
    return test_status;
}
