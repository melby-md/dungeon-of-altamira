#include <stdlib.h> // exit

#include "entity.h"
#include "gfx.h"
#include "stb_image.h"

static void resize(Graphics *);

void
Exit(Platform *p)
{
	// I'm not sure if this is necessary...
	SDL_DestroyTexture(p->graphics.font);
	SDL_DestroyRenderer(p->graphics.renderer);
	SDL_DestroyWindow(p->graphics.window);
	SDL_Quit();
}

void
WriteText(Graphics *g, str txt, int x, int y)
{
	int j = 0;
	for (int i = 0; i < txt.length; i++) {

		long c;

		// UTF-8 decode algo
		// TODO: handle overflows
		if (txt.data[i] < 0x80)
			c = (long)txt.data[i];
		else if ((txt.data[i] & 0xe0) == 0xc0) {
			c = ((long)(txt.data[i] & 0x1f) <<  6) |
			    ((long)(txt.data[i+1] & 0x3f) <<  0);
			i++;
		} else if ((txt.data[0] & 0xf0) == 0xe0) {
        		c = ((long)(txt.data[i] & 0x0f) << 12)   |
			    ((long)(txt.data[i+1] & 0x3f) <<  6) |
			    ((long)(txt.data[i+2] & 0x3f) <<  0);
			i += 2;
		} else if ((txt.data[0] & 0xf8) == 0xf0 && (txt.data[0] <= 0xf4)) {
			c = ((long)(txt.data[i] & 0x07) << 18)   |
			    ((long)(txt.data[i+1] & 0x3f) << 12) |
			    ((long)(txt.data[i+2] & 0x3f) <<  6) |
			    ((long)(txt.data[i+3] & 0x3f) <<  0);
			i += 3;
		} else // ignore invalid byte
			continue;
    		if (c >= 0xd800 && c <= 0xdfff)
			continue;

		SDL_Rect src = {(int)(c & (64 - 1)) * 8, (int)(c >> 6) * 16, 8, 16};
		SDL_Rect dst = {x + j * g->fontWidth, y, g->fontWidth, g->fontHeight};
		SDL_RenderCopy(g->renderer, g->font, &src, &dst);
		j++;
	};
}

static SDL_Texture *
LoadImage(Graphics *g, const char *filename)
{
	int width, height, channels;
	unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);

	int pitch;
	pitch = width * channels;
	pitch = (pitch + 3) & ~3;

	s32 red, green, blue, alpha;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	red = 0x000000FF;
	green = 0x0000FF00;
	blue = 0x00FF0000;
	alpha = (channels == 4) ? 0xFF000000 : 0;
#else
	int s = (channels == 4) ? 0 : 8;
	red = 0xFF000000 >> s;
	green = 0x00FF0000 >> s;
	blue = 0x0000FF00 >> s;
	alpha = 0x000000FF >> s;
#endif

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
		data, width, height, channels*8, pitch, red, green, blue, alpha
	);

	if (surface == NULL) {
		stbi_image_free(data);
		return NULL;
	}

	SDL_Texture *texture = SDL_CreateTextureFromSurface(g->renderer, surface);

	//SDL_FreeSurface(surface);
	//stbi_image_free(data);

	return texture;
}

SDL_Rect
TileTranslate(Graphics *g, float x, float y)
{
	return (SDL_Rect){(int)(x*(float)g->tile), (int)(y*(float)g->tile), g->tile, g->tile};
}

void
PollControls(Graphics *g)
{
	SDL_Event e = {0};

	while (SDL_PollEvent(&e)) {
		switch (e.type) {

		case SDL_QUIT:
			g->control.quit = 1;
			break;

		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
				g->height = e.window.data2;
				g->width = e.window.data1;
				resize(g);
			}
			break;

		case SDL_KEYDOWN: 
			switch (e.key.keysym.sym) {
			case SDLK_UP: case 'w': case 'k':
				g->control.up = 1;
				break;
			case SDLK_LEFT: case 'a': case 'h':
				g->control.left = 1;
				break;
			case SDLK_DOWN: case 's': case 'j':
				g->control.down = 1;
				break;
			case SDLK_RIGHT: case 'd': case 'l':
				g->control.right = 1;
				break;
			case SDLK_F5:
				g->control.reload = 1;
			}
			break;

		case SDL_KEYUP: 
			switch (e.key.keysym.sym) {
			case SDLK_UP: case 'w': case 'k':
				g->control.up = 0;
				break;
			case SDLK_LEFT: case 'a': case 'h':
				g->control.left = 0;
				break;
			case SDLK_DOWN: case 's': case 'j':
				g->control.down = 0;
				break;
			case SDLK_RIGHT: case 'd': case 'l':
				g->control.right = 0;
			}
		}
	}
}

static void
resize(Graphics *g)
{
	// TODO: find a better way to calculate this
	g->tile = g->height / 19;
	g->fontWidth = g->tile / 3;
	g->fontHeight = g->fontWidth * 2;

	g->wtiles = ((g->width - g->tile) / 2 - 1) / g->tile * 2 + 3;
	g->htiles = ((g->height - g->tile) / 2 - 1) / g->tile * 2 + 3;

	g->xoffset = ((g->wtiles - 1) * g->tile - (g->width - g->tile)) / 2;
	g->yoffset = ((g->htiles - 1) * g->tile - (g->height - g->tile)) / 2;
}

void
Render(Platform *platform, GameState *dungeon)
{
	Entity *player = dungeon->player.entity;

	// TODO: Rewrite this code
	float cx = player->sprite.x - ((float)platform->graphics.width / (float)platform->graphics.tile - 1.0f) / 2.0f;
	float cy = player->sprite.y - ((float)platform->graphics.height / (float)platform->graphics.tile - 1.0f) / 2.0f;

	int minx = (int)cx;
	int maxx = (int)(cx + (float)platform->graphics.width);

	int miny = (int)(cy);
	int maxy = (int)(cy + (float)platform->graphics.height);

	SDL_SetRenderDrawColor(platform->graphics.renderer, 0, 0, 0, 255);
	SDL_RenderClear(platform->graphics.renderer);

	for (int y = miny; y < maxy; y++) {
		for (int x = minx; x < maxx; x++) {
			if ((y < 0) || (y >= dungeon->h) || (x < 0) || (x >= dungeon->w))
				continue;

			if (!(dungeon->tiles[x + y * dungeon->w].flags & FOV))
				continue;
			SDL_Rect r = {
				.x = x*platform->graphics.tile - (int)(cx*(float)platform->graphics.tile),
				.y = y*platform->graphics.tile - (int)(cy*(float)platform->graphics.tile),
				.w = platform->graphics.tile,
				.h = platform->graphics.tile
			};

			switch (dungeon->tiles[x + y * dungeon->w].sprite) {
			case WALL:
				SDL_SetRenderDrawColor(platform->graphics.renderer, 64, 64, 64, 255);
				break;
			case FLOOR:
				SDL_SetRenderDrawColor(platform->graphics.renderer, 128, 128, 128, 255);
			}

			SDL_RenderFillRect(platform->graphics.renderer, &r);

		}
	}

	for (int i = 0; i < dungeon->entityCount; i++) {
		Entity *e = &dungeon->entities[i];

		if (e->x < minx || e->x > maxx || e->y < miny || e->y > maxy)
			continue;

		float x = e->sprite.x - cx;
		float y = e->sprite.y - cy;

		SDL_Rect r2 = TileTranslate(&platform->graphics, x, y);

		switch (e->sprite.id) {
		case PLAYER:
			SDL_SetRenderDrawColor(platform->graphics.renderer, 255, 255, 255, 255);
			break;
		case MONSTER:
			SDL_SetRenderDrawColor(platform->graphics.renderer, 255, 0, 0, 255);
		}
		SDL_RenderFillRect(platform->graphics.renderer, &r2);
	}

	str text = str("Nome: ");
	WriteText(&platform->graphics, text, 0, platform->graphics.height - platform->graphics.fontHeight);
	WriteText(&platform->graphics, dungeon->player.entity->name, (int)text.length*platform->graphics.fontWidth, platform->graphics.height - platform->graphics.fontHeight);

	SDL_RenderPresent(platform->graphics.renderer);
}

Platform *
InitPlatform(void)
{
	Platform *p;
	int winSize = 512;
	const char *errString = NULL;
	size arenaSize = 1<<28;
	// This pointer only exists do the message box in the error reporting
	SDL_Window *window = NULL;

	p = malloc(sizeof(*p) + sizeof(u8)*arenaSize);

	if (p == NULL) {
		errString = "OOM!";
		goto error;
	}

	p->arena.beg = p->memory;
	p->arena.end = p->arena.beg + arenaSize;

	SDL_memset(&p->graphics, 0, sizeof(p->graphics));

	// SDL startup
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		goto error;

	p->graphics.window = SDL_CreateWindow(
		"Dungeon of Altamira",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		winSize,
		winSize,
		0
	);

	if (p->graphics.window == NULL)
		goto error;

	window = p->graphics.window;

	p->graphics.renderer = SDL_CreateRenderer(p->graphics.window, -1, SDL_RENDERER_PRESENTVSYNC);

	if (p->graphics.renderer == NULL)
		goto error;

	SDL_GetWindowSize(p->graphics.window, &p->graphics.width, &p->graphics.height);

	resize(&p->graphics);

	p->graphics.font = LoadImage(&p->graphics, "./font.png");
	if (p->graphics.font == NULL) {
		errString = "Coudn't load font sprite";
		goto error;
	}

	return p;

error:
	if (errString == NULL)
		errString = SDL_GetError();

	SDL_ShowSimpleMessageBox(
		SDL_MESSAGEBOX_ERROR, "An error ocurred", errString, window
	);
	ErrorStr(errString);
	return NULL;
}
