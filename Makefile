CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lraylib -lm -lpthread -ldl -lrt -lX11

SRCS = main.c world.c particles.c renderer.c input.c
TARGET = sandbox

all:
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)
