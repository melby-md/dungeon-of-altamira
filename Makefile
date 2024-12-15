CC = cc -pipe

CPPFLAGS != sdl2-config --cflags
LDLIBS != sdl2-config --libs
LDLIBS += -ldl -lm

CFLAGS = -std=c11 -MD -O0 -g3 \
	 -fno-omit-frame-pointer $(WARNINGS)

WARNINGS = -Wall -Wextra -pedantic -Wvla -Wshadow -Wconversion \
	   -Wdouble-promotion -Wno-sign-conversion -Wno-unused-parameter \
	   -Wno-unused-variable -Wno-unused-function -Werror

DIR = out

-include config.mk

ifdef GL_ES
	CPPFLAGS += -DGL_ES
	LDLIBS += -lGLESv2
else
	LDLIBS += -lOpenGL
endif

OBJ = $(DIR)/arena.o $(DIR)/assets.o $(DIR)/game.o $(DIR)/platform_sdl2.o $(DIR)/renderer_gl.o $(DIR)/image.o

all: $(DIR)/dungeon_hotreload $(DIR)/dungeon.so

$(DIR)/dungeon_hotreload: src/unix_hotreload.c | $(DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -D'GAME_LIBRARY="./$(DIR)/dungeon.so"' $(LDFLAGS) -o $@ $< $(LDLIBS)

$(DIR)/dungeon.so: $(OBJ)
	$(CC) -shared -o $@ $(OBJ)

$(DIR)/%.o: src/%.c | $(DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -fPIC -c -o $@ $<

$(DIR)/image.o: WARNINGS = 

$(DIR):
	mkdir -p $@

clean:
	rm -rf $(DIR)

-include $(DIR)/*.d

.PHONY: clean
