CC = cc -pipe
CFLAGS = -std=c11 $$(sdl2-config --cflags)
LD_LIBS = -lm -ldl $$(sdl2-config --libs)

DEBUG_CFLAGS = $(CFLAGS) -Og -g3 -fsanitize=address,undefined \
	       -fno-omit-frame-pointer

WARNINGS = -Wall -Wextra -pedantic -Wvla -Wshadow \
	   -Wconversion -Wdouble-promotion -Wno-sign-conversion \
	   -Wno-unused-parameter -Werror

SRC = dungeon.c gfx.c arena.c entity.c
HDR = dungeon.h config.h gfx.h arena.h entity.h stb_image.h

OBJ = $(SRC:.c=.o) stb_image.o

all: dungeon_hotreload dungeon.so

dungeon_hotreload: unix_hotreload.c dungeon.h config.h
	$(CC) -o $@ $< $(DEBUG_CFLAGS) $(WARNINGS) $(LD_LIBS)

dungeon.so: $(OBJ)
	$(CC) -o $@ $(OBJ) -shared

.c.o:
	$(CC) -o $@ $< -fPIC -c $(DEBUG_CFLAGS) $(WARNINGS)

stb_image.o:
	$(CC) -o $@ -xc stb_image.h -DSTB_IMAGE_IMPLEMENTATION -fPIC -c $(DEBUG_CFLAGS)  

$(OBJ): $(HDR)

clean:
	rm -f dungeon $(OBJ) dungeon.so

.PHONY: all clean
