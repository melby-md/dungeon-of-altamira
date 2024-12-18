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

DIR ?= out

ifeq ($(OS),Windows_NT)
	USE_GLAD = 1
	HOT_RELOAD = 0
	EXE = .exe
endif

USE_GLAD ?= 1
HOT_RELOAD ?= 1
OBJ = $(DIR)/arena.o $(DIR)/assets.o $(DIR)/game.o $(DIR)/platform_sdl2.o $(DIR)/renderer_gl.o $(DIR)/image.o

ifeq ($(USE_GLAD),1)
	CPPFLAGS += -DGLAD
	OBJ += $(DIR)/glad.o
else
	ifeq ($(GL_ES),1)
		CPPFLAGS += -DGL_ES
		LDLIBS += -lGLESv2
	else
		LDLIBS += -lOpenGL
	endif
endif

ifeq ($(HOT_RELOAD),1)
	LDLIBS += -ldl
	TARGET = $(DIR)/dungeon_hotreload$(EXE) $(DIR)/dungeon.so
else
	TARGET = $(DIR)/dungeon$(EXE)
	OBJ += $(DIR)/main.o
endif

all: $(TARGET)

$(DIR)/dungeon_hotreload$(EXE): src/unix_hotreload.c | $(DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -D'GAME_LIBRARY="./$(DIR)/dungeon.so"' $(LDFLAGS) -o $@ $< $(LDLIBS)

$(DIR)/dungeon.so: $(OBJ)
	$(CC) -shared -o $@ $(OBJ)

$(DIR)/dungeon$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

$(DIR)/%.o: src/%.c | $(DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -fPIC -c -o $@ $<

$(DIR)/image.o: WARNINGS = 
$(DIR)/glad.o: WARNINGS =
$(DIR)/glad.o: third-party/glad.c 
	$(CC) $(CFLAGS) $(CPPFLAGS) -fPIC -c -o $@ $<

$(DIR):
	mkdir -p $@

clean:
	rm -rf $(DIR)

-include $(DIR)/*.d

.PHONY: clean
