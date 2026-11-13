CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99
LIBS ?= -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRCS = main.c world.c particles.c renderer.c input.c
TARGET = falling_sand

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
