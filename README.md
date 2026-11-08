# Falling Sand Simulator (C + raylib)

A small interactive pixel-material sandbox written in C.

## Features
- Sand falls, piles up, and displaces water.
- Water falls and spreads sideways.
- Stone is stationary.
- Fire rises, flickers through a few colors, and expires.
- Mouse painting, erasing, pause, reset, and brush sizing.

## Controls
- `1`: Sand
- `2`: Water
- `3`: Stone
- `4`: Fire
- `0`: Eraser
- Left mouse: paint selected material
- Right mouse: erase
- `Space`: pause / resume
- `R`: reset
- `[` and `]`: decrease / increase brush size

## Requirements
- A C compiler (GCC or Clang)
- raylib 5.x (or a compatible recent version)

## Build on Ubuntu / Debian
Install raylib using your distribution package if available:

```bash
sudo apt update
sudo apt install build-essential libraylib-dev
```

Then compile:

```bash
gcc main.c -o falling_sand -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./falling_sand
```

If your distribution's raylib package uses different linker flags, follow the
raylib installation instructions for your platform.

## Build on Windows
Install raylib and a supported compiler (for example, MinGW-w64). Use the
linker flags from the raylib Windows setup instructions. A typical command
with a MinGW raylib installation is:

```bash
gcc main.c -o falling_sand.exe -O2 -Wall -Wextra -std=c11 -I"path\\to\\raylib\\include" -L"path\\to\\raylib\\lib" -lraylib -lopengl32 -lgdi32 -lwinmm
```

## How it works
The world is a 160 x 120 grid. Each cell stores a material ID. Every frame,
the simulation scans from the bottom upward and applies simple local rules:
sand tries to move down or diagonally, water tries down and then sideways,
and fire drifts upward and decays.

This is a deliberately simple first version: it is a sandbox, not a physically
accurate fluid or combustion model.
