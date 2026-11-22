# Element Sandbox

A 2D particle-based sandbox simulation written in C using [raylib](https://www.raylib.com/). Paint sand, water, fire, oil and more, then watch them burn, flow, float and react.

**[▶ Play in your browser](https://vedanthkumar17.itch.io/element-sandbox)** (no download needed, works on desktop and mobile)

<!-- TODO: add a demo GIF here, e.g. ![Element Sandbox demo](docs/demo.gif) -->

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
- `F11`: Toggle fullscreen / windowed mode
- `[` and `]`: Decrease / Increase brush radius (1 to 12 px)

## How It Works

The world is a 160 × 120 grid of cells, each drawn as a 6 px square (960 × 720 window). Every frame, each cell is updated according to the rules of the material it holds, and a per-cell "already updated" flag stops a particle from moving twice in one frame.

Alongside the material grid, a separate temperature grid drives the fire and heat behavior: fire heats its neighbors, heat spreads between cells, materials ignite once they pass their threshold, and everything slowly cools back to the 20°C ambient temperature.

The game is written in C99 on top of raylib. The same source builds natively (Linux, Windows) and to WebAssembly via Emscripten, which is how the browser version is produced.

## Project Structure

```
element-sandbox/
├── src/
│   ├── common.h       # Grid/screen constants, temperature thresholds, ParticleType enum
│   ├── world.c/.h     # Grid state, cell swaps, temperature diffusion, world update
│   ├── particles.c/.h # Per-material physics and reactions (sand, water, fire, oil, ...)
│   ├── renderer.c/.h  # Cell colors, smoke transparency, HUD overlay
│   ├── input.c/.h     # Hotkeys, mouse painting, brush size
│   └── main.c         # Window setup and main loop
├── web/               # WebAssembly build output and HTML shell
├── Makefile           # Native and web build targets
├── LICENSE
└── README.md
```

## Build from Source

### Requirements
- A C compiler supporting C99 (GCC or Clang)
- `make`
- [raylib](https://www.raylib.com/) 5.x

### Linux (Ubuntu / Debian)

```bash
sudo apt update
sudo apt install build-essential libraylib-dev
make
./falling_sand
```

Or compile directly with `gcc`:
```bash
gcc src/main.c src/world.c src/particles.c src/renderer.c src/input.c -o falling_sand -Wall -Wextra -std=c99 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./falling_sand
```

### Windows (MinGW-w64)

```bash
gcc src/main.c src/world.c src/particles.c src/renderer.c src/input.c -o falling_sand.exe -O2 -Wall -Wextra -std=c99 -I"path\to\raylib\include" -L"path\to\raylib\lib" -lraylib -lopengl32 -lgdi32 -lwinmm
falling_sand.exe
```

### Web (WebAssembly via Emscripten)

Requirements:
- [Emscripten SDK (emsdk)](https://emscripten.org/), activated in your shell
- raylib compiled for `PLATFORM_WEB` (`libraylib.web.a`)

Build:
```bash
make web
```

This writes `index.html`, `index.js` and `index.wasm` into `web/`. Browsers refuse to load WebAssembly from `file://`, so opening `index.html` by double-clicking will not work. Serve the folder instead:

```bash
make serve
# or: python3 -m http.server 8000 --directory web
```

Then open `http://localhost:8000`.

To host your own copy (itch.io, GitHub Pages, any static host), upload the three files from `web/` together, with `index.html` at the root.

## License

Released under the [MIT License](LICENSE). raylib is licensed separately under the zlib license.
