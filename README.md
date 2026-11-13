# Falling Sand Simulator (C + raylib)

A modular, interactive pixel-material sandbox written in C using [raylib](https://www.raylib.com/).

## Features
- **Sand**: Falls, piles up at angles of repose, and displaces water.
- **Water**: Falls and spreads sideways, extinguishing fire and cooling cells.
- **Stone**: Stationary, impermeable barrier.
- **Fire**: Falls downward as embers when created, transitions to rising flickering flames upon landing or after a short delay, heats nearby cells, forms thermal plumes, and expires over time.
- **Grass**: Falls like a light powder, settles on stone/solids, spreads laterally across stone surfaces, and combusts when exposed to fire or temperatures >= 100°C.
- **Thermal System**: Models cell temperatures, thermal conduction, upward convection plumes, and gradual dissipation to ambient room temperature (20°C).
- **Interactive UI**: Real-time HUD showing material selection, brush size with visual indicator, hover temperature readout, palette shortcuts, pause indicator, and brush radius adjustment.

## Controls
- `1`: Select Sand
- `2`: Select Water
- `3`: Select Stone
- `4`: Select Fire
- `5`: Select Grass
- `0`: Eraser
- **Left Mouse Button (LMB)**: Paint selected material
- **Right Mouse Button (RMB)**: Erase material (clears to empty, resets temperature to ambient)
- `Space`: Pause / Resume simulation
- `R`: Reset canvas
- `[` and `]`: Decrease / Increase brush radius (1 to 12 px)

## Project Architecture & File Structure

The project is structured into clean, decoupled modules:

```
element-sandbox/
├── common.h         # Shared constants (grid sizes, temperatures) and ParticleType enum
├── world.h          # Grid storage declarations, swap helpers, and simulation interfaces
├── world.c          # Grid state allocation, cell manipulation, update_world(), temperature dissipation
├── particles.h      # Material-specific behavior declarations (try_sand, try_water, etc.)
├── particles.c      # Physics and interaction logic for all materials
├── renderer.h       # Visual rendering interface declarations
├── renderer.c       # World pixel rendering, material colors, and HUD/UI drawing
├── input.h          # User input and brush state interface declarations
├── input.c          # Keyboard shortcuts, mouse painting, and brush management
├── main.c           # Window creation, initialization, and main game loop
├── Makefile         # Build automation for compilation and cleanup
└── README.md        # Project documentation and guide
```

### Module Responsibilities

- **`common.h`**: Defines grid dimensions (`GRID_W = 160`, `GRID_H = 120`, `CELL = 6`), screen dimensions, ambient & ignition temperatures, and the `ParticleType` enum (`EMPTY`, `SAND`, `WATER`, `STONE`, `FIRE`, `GRASS`).
- **`world.h` / `world.c`**: Owns the 2D simulation arrays (`world`, `updated`, `fire_life`, `fire_falling`, `grass_timer`, `temperature`), bounds-checking (`inside`), empty checks (`empty_at`), atomic cell swapping (`swap_cells`), global grid resetting (`reset_world`), and temperature dissipation (`update_temperature`).
- **`particles.h` / `particles.c`**: Encapsulates material rules:
  - `try_sand()`: gravity, sliding diagonally, displacing water, burying grass.
  - `try_water()`: downward flow and multi-step horizontal spread.
  - `try_fire()`: water quenching, thermal plume generation, falling ember vs. rising flame phases, flammable ignition.
  - `try_grass()`: thermal & contact ignition, light powder falling, lateral growth on stone.
- **`renderer.h` / `renderer.c`**: Draws the world grid pixels with material color variations, and renders the translucent HUD showing active material, brush preview circle, hover cell temperature, palette hotkeys, and pause banner.
- **`input.h` / `input.c`**: Processes keyboard inputs (hotkeys 0-5, pause toggle, canvas reset, brush resize) and mouse painting with circular brush stamp.
- **`main.c`**: Minimal coordinator; initializes raylib, world, and input systems, runs the 60 FPS loop, and handles clean teardown.

---

## Requirements
- A C compiler supporting C11 (GCC or Clang)
- `make` (optional, for build automation)
- [raylib](https://www.raylib.com/) 5.x development libraries

---

## Build and Run

### Ubuntu / Debian

Install build essentials and raylib:
```bash
sudo apt update
sudo apt install build-essential libraylib-dev
```

Compile using `make`:
```bash
make
./falling_sand
```

Or compile directly with `gcc`:
```bash
gcc -Wall -Wextra -std=c11 -O2 main.c world.c particles.c renderer.c input.c -o falling_sand -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./falling_sand
```

### macOS

Install raylib via Homebrew:
```bash
brew install raylib
```

Compile:
```bash
gcc -Wall -Wextra -std=c11 -O2 main.c world.c particles.c renderer.c input.c -o falling_sand -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
./falling_sand
```

### Windows (MinGW-w64)

Compile using GCC:
```bash
gcc -Wall -Wextra -std=c11 -O2 main.c world.c particles.c renderer.c input.c -o falling_sand.exe -I"path\to\raylib\include" -L"path\to\raylib\lib" -lraylib -lopengl32 -lgdi32 -lwinmm
falling_sand.exe
```

---

## How to Add a New Material

The multi-file structure makes adding a new material straightforward:

1. **Add enum value** in `common.h`:
   ```c
   typedef enum {
       EMPTY = 0,
       SAND,
       WATER,
       STONE,
       FIRE,
       GRASS,
       OIL // <- New material
   } ParticleType;
   ```

2. **Implement movement / reaction logic** in `particles.h` and `particles.c`:
   - Declare `void try_oil(int x, int y);` in `particles.h`.
   - Implement `try_oil(int x, int y)` in `particles.c` using `swap_cells`, `empty_at`, and `inside`.

3. **Register in simulation step** in `world.c`:
   Add a case in `update_world()`:
   ```c
   case OIL: try_oil(x, y); break;
   ```

4. **Define visual colors and UI representation** in `renderer.c`:
   - Add a case in `material_color()` for in-world pixel colors.
   - Add a case in `material_name()` and `material_ui_color()` for HUD rendering.
   - Add an entry to `palette_items[]` in `draw_ui()`.

5. **Add input selection** in `input.c`:
   - Add a hotkey in `handle_input()` (e.g. `if (IsKeyPressed(KEY_SIX)) selected = OIL;`).
   - If the material requires specific initialization when painted (e.g., custom temperature or life timer), add it to `paint_at_mouse()`.
