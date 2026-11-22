# Element Sandbox Makefile
# Supports both native Linux desktop builds and Emscripten WebAssembly (HTML5) builds

PLATFORM ?= PLATFORM_DESKTOP

# Directories
SRC_DIR ?= src
BUILD_DIR ?= build
WEB_DIR ?= web

# Source files
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/world.c $(SRC_DIR)/particles.c $(SRC_DIR)/renderer.c $(SRC_DIR)/input.c
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# ---------------------------------------------------------
# Native Desktop Configuration
# ---------------------------------------------------------
CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99
INCLUDES = -I$(SRC_DIR)
LIBS ?= -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
TARGET = falling_sand

# ---------------------------------------------------------
# Web (Emscripten / WebAssembly) Configuration
# ---------------------------------------------------------
WEB_TARGET = $(WEB_DIR)/index.html
SHELL_FILE ?= $(WEB_DIR)/shell.html
PORT ?= 8000

# Locate EMSDK / emcc
EMSDK ?= $(firstword $(wildcard $(HOME)/emsdk /home/vedanthkumar17/emsdk))
ifneq ($(shell which emcc 2>/dev/null),)
    EMCC = emcc
else ifneq ($(wildcard $(EMSDK)/upstream/emscripten/emcc),)
    EMCC = $(EMSDK)/upstream/emscripten/emcc
else
    EMCC = emcc
endif

# Locate Raylib source / web library
RAYLIB_PATH ?= $(firstword $(wildcard $(HOME)/raylib /home/vedanthkumar17/raylib ../raylib ./raylib /usr/local))
RAYLIB_WEB_INCLUDE ?= $(RAYLIB_PATH)/src
RAYLIB_WEB_LIB ?= $(RAYLIB_PATH)/src/libraylib.web.a

EMCFLAGS ?= -Os -Wall -Wextra -std=gnu99 -D_DEFAULT_SOURCE -DPLATFORM_WEB -I$(SRC_DIR) -I$(RAYLIB_WEB_INCLUDE)
EMLDFLAGS ?= -s USE_GLFW=3 -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=0 -s EXPORTED_RUNTIME_METHODS=ccall --shell-file $(SHELL_FILE)

# ---------------------------------------------------------
# Targets
# ---------------------------------------------------------
ifeq ($(PLATFORM),PLATFORM_WEB)
all: web
else
all: $(TARGET)
endif

# Native compilation
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

# Build raylib web archive if not yet compiled
$(RAYLIB_WEB_LIB):
	@echo "Building raylib for WebAssembly (PLATFORM_WEB)..."
	$(MAKE) -C $(RAYLIB_PATH)/src PLATFORM=PLATFORM_WEB

# Web build target
web: $(WEB_TARGET)

$(WEB_TARGET): $(SRCS) $(SHELL_FILE) $(RAYLIB_WEB_LIB)
	@mkdir -p $(WEB_DIR)
	$(EMCC) $(SRCS) $(RAYLIB_WEB_LIB) -o $(WEB_TARGET) $(EMCFLAGS) $(EMLDFLAGS)
	@echo "Web build complete: $(WEB_DIR)/ (index.html, index.js, index.wasm)"

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -f $(WEB_DIR)/index.html $(WEB_DIR)/index.js $(WEB_DIR)/index.wasm

run: $(TARGET)
	./$(TARGET)

serve: web
	@echo "Starting local web server at http://localhost:$(PORT)..."
	python3 -m http.server $(PORT) --directory $(WEB_DIR)

.PHONY: all clean run web serve
