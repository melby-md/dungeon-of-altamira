#ifndef _DUNGEON_H
#define _DUNGEON_H
#include "config.h"
#include "arena.h"
#include "dungeon.h"

typedef struct Controls {
	unsigned up     : 1;
	unsigned down   : 1;
	unsigned left   : 1;
	unsigned right  : 1;
	unsigned quit   : 1;
	unsigned reload : 1;
} Controls;

typedef struct Graphics {
	int width, height;
	int wtiles, htiles;
	int xoffset, yoffset;
	int fontWidth, fontHeight;
	int tile;

	u32 lastFrame;

	Controls control;
	SDL_Renderer *renderer;
	SDL_Window *window;
	SDL_Texture *font;
} Graphics;

struct Platform {
	Graphics graphics;
	Arena arena;
	u8 memory[];
};

SDL_Rect TileTranslate(Graphics *, float x, float y);
void PollControls(Graphics *);
void Exit(Platform *);
Platform *InitPlatform(void);
void WriteText(Graphics *, string, int, int);
void Render(Platform *, GameState *);
#endif
