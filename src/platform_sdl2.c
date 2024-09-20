#include <stdbool.h>
#include <stdlib.h> // malloc

#include "config.h"
#include "game.h"
#include "hotreload.h"
#include "platform.h"
#include "renderer_gl.h"
#include "SDL.h"

struct Game {
	SDL_Window *window;
	SDL_GLContext gl_ctx;
	int width, height;
	Renderer renderer;
	u64 lastUpdate;
	int keyboard_length;
	const u8 *keyboard_state;

	GameState *state;
	u8 memory[];
};

void Finalize(Game *game)
{
	SDL_GL_DeleteContext(game->gl_ctx);
	SDL_DestroyWindow(game->window);
	SDL_Quit();
}

void Exit(int s)
{
	exit(s);
}

#if 0
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
#endif

static void resize(Game *game)
{
	int width, height;
	SDL_GL_GetDrawableSize(game->window, &width, &height);
	Log("Resized: %dx%d", width, height);

	RendererResize(&game->renderer, width, height);

	game->width = width;
	game->height = height;
}

void Loop(Game *game)
{
	Controls controls = {0};
	SDL_Event e = {0};
	while (SDL_PollEvent(&e)) {
		switch (e.type) {

		case SDL_QUIT:
			GAME_API.running = false;
			break;

		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
				resize(game);
			}
			break;
		case SDL_KEYDOWN: 
			if (e.key.keysym.sym == SDLK_F5)
				LoadShaders(&game->renderer);
		}

	}

	//if (game->keyboard_state[SDL_SCANCODE_F5])
	//	InitGame(game->state);
	
	controls.up    = game->keyboard_state[SDL_SCANCODE_W] || game->keyboard_state[SDL_SCANCODE_UP];
	controls.left  = game->keyboard_state[SDL_SCANCODE_A] || game->keyboard_state[SDL_SCANCODE_LEFT];
	controls.down  = game->keyboard_state[SDL_SCANCODE_S] || game->keyboard_state[SDL_SCANCODE_DOWN];
	controls.right = game->keyboard_state[SDL_SCANCODE_D] || game->keyboard_state[SDL_SCANCODE_RIGHT];

	u64 current = SDL_GetPerformanceCounter();
	float elapsedTime = (float)(current - game->lastUpdate) / (float)SDL_GetPerformanceFrequency();
	game->lastUpdate = current;

	Update(game->state, controls, elapsedTime);

	Render(game->state, &game->renderer);

	RendererFlush(&game->renderer);
	SDL_GL_SwapWindow(game->window);
}

Game *InitPlatform(void)
{
	Game *game;
	const char *errString = NULL;
	size memorySize = 1<<28;
	SDL_Window *window = NULL;
	int width = 512, height = 512;

	SDL_version compiled;
	SDL_version linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);

	Log(
		"Compiled Against SDL Version: %u.%u.%u.\n",
		compiled.major, compiled.minor, compiled.patch
	);
	Log(
		"SDL Version Loaded: %u.%u.%u.",
		linked.major, linked.minor, linked.patch
	);

	Log("Platform: %s", SDL_GetPlatform());

	game = malloc(sizeof(Game) + memorySize);

	if (game == NULL) {
		errString = "OOM!";
		goto error;
	}
	
	// SDL startup
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		goto error;

	Log("Video Driver: %s", SDL_GetCurrentVideoDriver());

#ifdef GL_ES
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

	window = SDL_CreateWindow(
		"Dungeon of Altamira",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_OPENGL
	);

	if (window == NULL)
		goto error;

	game->keyboard_state = SDL_GetKeyboardState(&game->keyboard_length);

	game->window = window;
	game->gl_ctx = SDL_GL_CreateContext(window);
	if (game->gl_ctx == NULL)
		goto error;
#ifndef GL_ES
	if (SDL_GL_ExtensionSupported("GL_KHR_debug"))
		RendererEnableDebugLogs();
#endif

	resize(game);
	RendererInit(&game->renderer);

	game->lastUpdate = SDL_GetPerformanceCounter();
	game->state = InitGame(game->memory, memorySize, &game->renderer);

	return game;

error:
	if (errString == NULL)
		errString = SDL_GetError();

	SDL_ShowSimpleMessageBox(
		SDL_MESSAGEBOX_ERROR, "An error ocurred", errString, window
	);
	ErrorStr(errString);
	return NULL;
}

str ReadEntireFile(const char *name)
{
	str out;
	out.data = SDL_LoadFile(name, (size_t *)&out.length);
	return out;
}

GameApi GAME_API = {
	.init = InitPlatform,
	.loop = Loop,
	.finalize = Finalize,
	.running = true,
};
