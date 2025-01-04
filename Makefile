CC = cc -pipe

-include config.mk

SDL_CONFIG ?= sdl2-config
SDL_CPPFLAGS != $(SDL_CONFIG) --cflags
SDL_LDLIBS != $(SDL_CONFIG) --libs
LDLIBS += -lm $(SDL_LDLIBS)
CPPFLAGS += $(SDL_CPPFLAGS) -Ithird-party

CFLAGS ?= -std=c11 -MD -O0 -g3 \
	 -fno-omit-frame-pointer $(WARNINGS)

WARNINGS ?= -Wall -Wextra -pedantic -Wvla -Wshadow -Wconversion \
	   -Wdouble-promotion -Wno-sign-conversion -Wno-unused-parameter \
	   -Wno-unused-variable -Wno-unused-function -Werror

ifeq ($(OS),Windows_NT)
	EXE = .exe
endif

GLVND ?= 0

OBJ = out/arena.o out/assets.o out/game.o out/platform_sdl2.o out/renderer_gl.o out/image.o out/glad.o

ifeq ($(GL_ES),1)
	CPPFLAGS += -DGL_ES
endif

ifeq ($(GLVND),1)
	ifeq ($(GL_ES),1)
		LDLIBS += -lGLESv2
	else
		LDLIBS += -lOpenGL
	endif
else
	CPPFLAGS += -DGLAD
endif


all: out/dungeon$(EXE)

out/dungeon$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

out/%.o: src/%.c | out
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

out/image.o: WARNINGS = 
out/glad.o: WARNINGS =
out/glad.o: third-party/gl.c | out
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

out:
	mkdir -p $@

clean:
	rm -rf out

-include out/*.d

.PHONY: clean all
