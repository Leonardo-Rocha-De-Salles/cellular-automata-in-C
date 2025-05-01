CFLAGS=-std=c17 -Wall -Wextra -Werror -g
all:
	gcc sand.c button.c -o sand $(CFLAGS) `sdl2-config --cflags --libs`
