#include "renderer.h"
#include "world.h"
#include "input.h"
#include "common.h"
#include "raylib.h"

Color material_color(int material, int x, int y) {
    switch (material) {
        case SAND:
            return (GetRandomValue(0, 8) == 0)
                ? (Color){246, 210, 118, 255}
                : (Color){226, 184, 94, 255};
        case WATER:
            return (Color){40, 140 + (unsigned char)((x + y) % 35), 245, 255};
        case GRAVEL:
            return (Color){125, 132, 144, 255};
        case FIRE: {
            int life = fire_life[y][x];
            return life > 45 ? (Color){255, 235, 80, 255}
                 : life > 20 ? (Color){255, 130, 30, 255}
                             : (Color){230, 50, 25, 255};
        }
        case GRASS:
            return (Color){72, 195, 82, 255};
        case WOOD:
            if (wood_health[y][x] > 0) {
                // Burning wood with glowing ember highlights
                return (GetRandomValue(0, 1) == 0)
                    ? (Color){255, 115, 35, 255}
                    : (Color){185, 65, 25, 255};
            }
            // Natural wood grain pattern
            return ((x + y / 2) % 6 == 0)
                ? (Color){118, 75, 38, 255}
                : (((x + y) % 3 == 0) ? (Color){145, 95, 50, 255} : (Color){132, 84, 44, 255});
        case ASH:
            return ((x * 7 + y * 13) % 5 == 0)
                ? (Color){188, 188, 192, 255}
                : (((x + y) % 2 == 0) ? (Color){164, 164, 170, 255} : (Color){145, 145, 150, 255});
        case SMOKE: {
            int life = smoke_life[y][x];
            unsigned char alpha = (unsigned char)(life > 75 ? 190 : (life * 2 + 40));
            return (Color){115, 118, 128, alpha};
        }
        case OIL:
            return ((x + y) % 4 == 0)
                ? (Color){58, 50, 38, 255}
                : (((x * 3 + y) % 5 == 0) ? (Color){32, 28, 22, 255} : (Color){46, 40, 32, 255});
        default:
            return BLANK;
    }
}

void draw_world(void) {
    ClearBackground((Color){6, 6, 8, 255});

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            int material = world[y][x];
            if (material == EMPTY) {
                continue;
            }

            DrawRectangle(x * CELL, y * CELL, CELL, CELL,
                          material_color(material, x, y));
        }
    }
}

const char *material_name(int material) {
    switch (material) {
        case SAND:  return "SAND";
        case WATER: return "WATER";
        case GRAVEL: return "GRAVEL";
        case FIRE:  return "FIRE";
        case GRASS: return "GRASS";
        case WOOD:  return "WOOD";
        case ASH:   return "ASH";
        case SMOKE: return "SMOKE";
        case OIL:   return "OIL";
        default:    return "ERASER";
    }
}

Color material_ui_color(int material) {
    switch (material) {
        case SAND:  return (Color){240, 200, 105, 255};
        case WATER: return (Color){45, 150, 245, 255};
        case GRAVEL: return (Color){125, 132, 144, 255};
        case FIRE:  return (Color){255, 130, 30, 255};
        case GRASS: return (Color){72, 195, 82, 255};
        case WOOD:  return (Color){145, 95, 50, 255};
        case ASH:   return (Color){175, 175, 182, 255};
        case SMOKE: return (Color){150, 155, 165, 255};
        case OIL:   return (Color){68, 60, 48, 255};
        default:    return (Color){60, 64, 75, 255};
    }
}

void draw_ui(void) {
    int selected = get_selected_material();
    int brush_radius = get_brush_radius();
    bool paused = is_paused();

    // Semi-transparent dark rounded container
    Rectangle panel = { 10, 10, 720, 76 };
    DrawRectangleRounded(panel, 0.16f, 8, (Color){ 12, 14, 18, 225 });
    DrawRectangleRoundedLinesEx(panel, 0.16f, 8, 1.0f, (Color){ 45, 52, 65, 220 });

    // Primary row: Material swatch + name and Brush indicator
    Rectangle swatch = { 20, 19, 16, 16 };
    DrawRectangleRounded(swatch, 0.25f, 4, material_ui_color(selected));
    DrawRectangleRoundedLinesEx(swatch, 0.25f, 4, 1.0f,
        (selected == EMPTY) ? (Color){ 200, 80, 80, 220 } : (Color){ 255, 255, 255, 70 });

    if (selected == EMPTY) {
        DrawLine(swatch.x + 4, swatch.y + 4, swatch.x + swatch.width - 4, swatch.y + swatch.height - 4, (Color){ 220, 80, 80, 255 });
        DrawLine(swatch.x + swatch.width - 4, swatch.y + 4, swatch.x + 4, swatch.y + swatch.height - 4, (Color){ 220, 80, 80, 255 });
    }

    DrawText(material_name(selected), 44, 18, 18, RAYWHITE);

    // Subtle divider
    DrawRectangle(165, 20, 1, 15, (Color){ 45, 52, 65, 255 });

    // Brush info & visual preview
    DrawText("Brush:", 178, 20, 14, (Color){ 150, 158, 172, 255 });
    DrawText(TextFormat("%d px", brush_radius), 228, 18, 17, RAYWHITE);

    // Visual brush circle indicator
    DrawCircleLines(292, 27, 9.0f, (Color){ 65, 72, 88, 200 });
    float preview_r = (float)brush_radius * 0.65f + 1.0f;
    if (preview_r > 8.0f) preview_r = 8.0f;
    DrawCircle(292, 27, preview_r, (Color){ 215, 225, 240, 220 });

    DrawText("[ / ]", 310, 20, 12, (Color){ 120, 128, 142, 255 });

    // Cursor temperature readout
    Vector2 mouse = GetMousePosition();
    int mx = (int)mouse.x / CELL;
    int my = (int)mouse.y / CELL;
    if (inside(mx, my)) {
        float t = temperature[my][mx];
        Color temp_col = (t >= GRASS_IGNITE_TEMP) ? (Color){ 255, 120, 60, 255 }
                       : (t > 40.0f)              ? (Color){ 245, 195, 90, 255 }
                                                  : (Color){ 145, 185, 230, 220 };
        DrawText(TextFormat("%d C", (int)t), 365, 19, 14, temp_col);
    }

    // Secondary row: Material palette hotkeys with active indicator
    static const struct {
        int id;
        const char *label;
    } palette_items[] = {
        { SAND,  "1 Sand" },
        { WATER, "2 Water" },
        { GRAVEL, "3 Gravel" },
        { FIRE,  "4 Fire" },
        { GRASS, "5 Grass" },
        { WOOD,  "6 Wood" },
        { ASH,   "7 Ash" },
        { SMOKE, "8 Smoke" },
        { OIL,   "9 Oil" },
        { EMPTY, "0 Eraser" }
    };

    int cur_x = 20;
    for (int i = 0; i < 10; i++) {
        int text_w = MeasureText(palette_items[i].label, 12);
        bool is_sel = (selected == palette_items[i].id);
        if (is_sel) {
            DrawRectangleRounded((Rectangle){ cur_x - 4, 40, text_w + 8, 15 }, 0.35f, 4, (Color){ 255, 255, 255, 36 });
            DrawText(palette_items[i].label, cur_x, 42, 12, RAYWHITE);
        } else {
            DrawText(palette_items[i].label, cur_x, 42, 12, (Color){ 135, 142, 156, 255 });
        }
        cur_x += text_w + 10;
    }

    // Tertiary row: Keybindings & mouse controls
    DrawText("SPACE Pause  |  R Reset  |  RMB Erase  |  LMB Paint  |  [ / ] Brush Size",
             20, 61, 11, (Color){ 105, 112, 126, 255 });

    // Paused indicator banner
    if (paused) {
        int pw = 100;
        int ph = 26;
        int px = SCREEN_W / 2 - pw / 2;
        int py = 12;
        DrawRectangleRounded((Rectangle){ px, py, pw, ph }, 0.4f, 6, (Color){ 180, 42, 42, 225 });
        DrawRectangleRoundedLinesEx((Rectangle){ px, py, pw, ph }, 0.4f, 6, 1.0f, (Color){ 245, 95, 95, 200 });

        DrawRectangle(px + 16, py + 7, 3, 12, RAYWHITE);
        DrawRectangle(px + 22, py + 7, 3, 12, RAYWHITE);
        DrawText("PAUSED", px + 33, py + 6, 15, RAYWHITE);
    }
}
