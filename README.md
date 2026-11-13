# Falling Sand Simulator (C + raylib)

A modular, interactive pixel-material sandbox written in C using [raylib](https://www.raylib.com/).

## Features & Materials

### Materials
1. **Sand (`1`)**: Falls under gravity, forms natural angular piles, and displaces lighter liquids (water, oil) and ash.
2. **Water (`2`)**: Falls and spreads sideways. Quenches regular fires, cools heated cells, and is denser than oil (sinks beneath it).
3. **Gravel (`3`)**: Coarse mineral particles. Falls and piles up like sand, sinks through liquids, and forms a solid foundation for other materials (like grass) to rest on.
4. **Fire (`4`)**: Falls initially as hot embers, transitions to flickering rising flames, generates upward thermal convection plumes, and combusts flammable materials.
5. **Grass (`5`)**: Falls like a light powder, settles on solid surfaces, spreads laterally across gravel and wood, and combusts when heated or touched by fire.
6. **Wood (`6`)**: Solid structural material. Ignites from direct contact or temperatures >= 200°C. Burns gradually, radiating heat, emitting leaping flames and rising smoke, before crumbling into ash.
7. **Ash (`7`)**: Fine powder residue created from burning wood and grass. Falls and heaps like sand; slowly settles into water.
8. **Smoke (`8`)**: Lightweight gas produced by combustion. Rises upward, floats up through liquids, billows sideways, and gradually dissipates.
9. **Oil (`9`)**: Viscous liquid that falls and spreads. Less dense than water, causing it to float on top of water surfaces. Highly flammable—ignites at >= 70°C, burns longer than normal fire with intense heat, and generates thick smoke plumes.
10. **Eraser (`0`)**: Clears cells back to empty space and restores ambient temperature.

### Material Reactions & Interactions
- **Fire + Grass**: Grass ignites violently, creating leaping flames, rising smoke, and leaving behind ash.
- **Fire + Wood**: Wood smolders and burns with visual embers, generating roaring fire and smoke before degrading into falling ash.
- **Oil + Water**: Oil floats on top of water pools; water poured on oil sinks straight through.
- **Fire + Oil**: Oil catches fire easily and burns longer with hot flames (520°C) and billowing dark smoke. Burning oil floating on water remains ignited on the surface.
- **Water + Fire / Burning Wood**: Water quenches flames and cools smoldering wood.
- **Thermal System**: Models conduction, thermal plumes, ignition thresholds, and ambient cooling (20°C).

## Controls
- `1`: Select Sand
- `2`: Select Water
- `3`: Select Gravel
- `4`: Select Fire
- `5`: Select Grass
- `6`: Select Wood
- `7`: Select Ash
- `8`: Select Smoke
- `9`: Select Oil
- `0`: Eraser (Empty)
- **Left Mouse Button (LMB)**: Paint selected material
- **Right Mouse Button (RMB)**: Erase material (clears to empty, resets temperature)
- `Space`: Pause / Resume simulation
- `R`: Reset world canvas
- `[` and `]`: Decrease / Increase brush radius (1 to 12 px)

## Project Architecture & File Structure

The project is structured into clean, decoupled modules:

```
element-sandbox/
├── common.h         # Shared constants (grid sizes, temperatures) and ParticleType enum
├── world.h          # Grid storage declarations, swap helpers, and simulation interfaces
├── world.c          # Grid state allocation, cell manipulation, update_world(), temperature dissipation
├── particles.h      # Material-specific behavior declarations (try_sand, try_gravel, try_oil, etc.)
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

- **`common.h`**: Defines grid dimensions (`GRID_W = 160`, `GRID_H = 120`, `CELL = 6`), screen dimensions (`960x720`), ambient & ignition thresholds (`FIRE_TEMP`, `GRASS_IGNITE_TEMP`, `WOOD_IGNITE_TEMP`, `OIL_IGNITE_TEMP`), and the complete `ParticleType` enum.
- **`world.h` / `world.c`**: Owns simulation grid arrays (`world`, `updated`, `fire_life`, `fire_falling`, `grass_timer`, `smoke_life`, `wood_health`, `leaves_ash`, `temperature`), boundary checks (`inside`), cell swaps (`swap_cells`), and ambient temperature diffusion (`update_temperature`).
- **`particles.h` / `particles.c`**: Encapsulates material physics:
  - `try_sand()`: gravity, slope sliding, displacing water/oil/ash, burying grass.
  - `try_water()`: gravity, lateral flow, sinking beneath lighter oil.
  - `try_gravel()`: coarse-particle gravity, piling at angles of repose, sinking through liquids.
  - `try_fire()`: water quenching, thermal plumes, combustion of grass, wood, and oil.
  - `try_grass()`: combustion into Fire + Smoke + Ash, powder falling, lateral growth on gravel/wood.
  - `try_wood()`: solid stability, burning stages, fire & smoke emission, crumbling into ash.
  - `try_ash()`: fine powder gravity and piling.
  - `try_smoke()`: buoyant upward drift, liquid bubbling, lifetime dissipation.
  - `try_oil()`: liquid flow, floating on water, rapid cascading flammability, long burn life.
- **`renderer.h` / `renderer.c`**: Renders particle pixel colors with natural texture variations, burning embers, translucent smoke alpha, and the interactive HUD overlay.
- **`input.h` / `input.c`**: Handles 1–9 and 0 material hotkeys, pause toggle, canvas reset, brush resizing, and circular mouse painting.
- **`main.c`**: Minimal application entry point and 60 FPS update loop.

---

## Requirements
- A C compiler supporting C99 or C11 (GCC or Clang)
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

Compile and run using `make`:
```bash
make
./falling_sand
```

Or compile directly with `gcc`:
```bash
gcc main.c world.c particles.c renderer.c input.c -o falling_sand -Wall -Wextra -std=c99 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./falling_sand
```

### Windows (MinGW-w64)

Compile using GCC:
```bash
gcc main.c world.c particles.c renderer.c input.c -o falling_sand.exe -O2 -Wall -Wextra -std=c99 -I"path\to\raylib\include" -L"path\to\raylib\lib" -lraylib -lopengl32 -lgdi32 -lwinmm
falling_sand.exe
```
