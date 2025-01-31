#include <stdbool.h>
#include <stdlib.h> // malloc

#ifdef GLAD
#  include <glad/gl.h>
#endif

#include "common.h"
#include "game.h"
#include "hotreload.h"
#include "platform.h"
#include "renderer_gl.h"
#include "SDL.h"

struct Game {
	SDL_Window *window;
	SDL_GLContext gl_ctx;
	Renderer renderer;
	u64 lastUpdate;
	int keyboard_length;
	const u8 *keyboard_state;
	bool fullscreen;

	GameState *state;
	char memory[];
};

static bool running = true;

void Finalize(Game *game)
{
	SDL_GL_DeleteContext(game->gl_ctx);
	SDL_DestroyWindow(game->window);
	SDL_Quit();
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
}

void Loop(Game *game)
{
	Controls controls = {0};
	SDL_Event e = {0};
	while (SDL_PollEvent(&e)) {
		switch (e.type) {

		case SDL_QUIT:
			RequestExit();
			break;

		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
				resize(game);
			}
			break;
		case SDL_KEYDOWN: 
			if (e.key.keysym.sym == SDLK_F11) {
				SDL_SetWindowFullscreen(
					game->window,
					game->fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP
				);
				game->fullscreen = !game->fullscreen;
			}
		}
	}
	
	u8 up    = game->keyboard_state[SDL_SCANCODE_W] || game->keyboard_state[SDL_SCANCODE_UP];
	u8 left  = game->keyboard_state[SDL_SCANCODE_A] || game->keyboard_state[SDL_SCANCODE_LEFT];
	u8 down  = game->keyboard_state[SDL_SCANCODE_S] || game->keyboard_state[SDL_SCANCODE_DOWN];
	u8 right = game->keyboard_state[SDL_SCANCODE_D] || game->keyboard_state[SDL_SCANCODE_RIGHT];

	Vec2 unnormalized = (Vec2){
		(float)(right - left),
		(float)(down - up)
	};

	if (unnormalized.x != 0.f || unnormalized.y != 0.f)
		controls.direction = Vec2Normalize(unnormalized);

	u64 current = SDL_GetPerformanceCounter();
	float dt = (float)(current - game->lastUpdate) / (float)SDL_GetPerformanceFrequency();
	game->lastUpdate = current;

	Update(game->state, controls, dt);

	BeginRender(&game->renderer);
	Render(game->state, &game->renderer);
	EndRender(&game->renderer);

	SDL_GL_SwapWindow(game->window);
}

NORETURN void Panic(const char *msg)
{
	ErrorStr(msg);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "An error ocurred", msg, NULL);
	exit(1);
}

void RequestExit(void)
{
	running = false;
}

Game *InitPlatform(void)
{
	Game *game;
	const char *errString = NULL;
	size memorySize = 1<<28;
	SDL_Window *window = NULL;
	int width = 640, height = 360;

	SDL_version compiled;
	SDL_version linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);

	Log(
		"Compiled Against SDL Version: %u.%u.%u.",
		compiled.major, compiled.minor, compiled.patch
	);
	Log(
		"SDL Version Loaded: %u.%u.%u.",
		linked.major, linked.minor, linked.patch
	);

	Log("Platform: %s", SDL_GetPlatform());

	game = malloc(sizeof(Game) + memorySize);

	if (game == NULL)
		Panic("OOM!");

	game->fullscreen = false;
	
	// SDL startup
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		Panic(SDL_GetError());

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

#ifdef DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	u32 window_flags = SDL_WINDOW_OPENGL;
	if (game->fullscreen)
		window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

	game->window = SDL_CreateWindow(
		"Dungeon of Altamira",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		window_flags
	);

	if (game->window == NULL)
		Panic(SDL_GetError());

	game->keyboard_state = SDL_GetKeyboardState(&game->keyboard_length);

	game->gl_ctx = SDL_GL_CreateContext(game->window);
	if (game->gl_ctx == NULL)
		Panic(SDL_GetError());

#ifdef GLAD
#  ifdef GL_ES
	gladLoadGLES2((GLADloadfunc)SDL_GL_GetProcAddress);
#  else
	gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
#  endif
#endif

	SDL_GL_SetSwapInterval(1);

	Arena arena;
	ArenaInit(&arena, game->memory, memorySize);

	bool gl_debug = false;
#ifdef DEBUG
	gl_debug = SDL_GL_ExtensionSupported("GL_KHR_debug");
#endif

	RendererInit(&game->renderer, arena, gl_debug);
	resize(game);

	game->lastUpdate = SDL_GetPerformanceCounter();
	game->state = InitGame(&arena, &game->renderer);

	return game;
}

Str ReadEntireFile(const char *name)
{
	Str out;
	out.data = SDL_LoadFile(name, (size_t *)&out.length);
	return out;
}

int main(int argc, char *argv[]) {
	Game *game = InitPlatform();
	while (running) {
		Loop(game);
	}
	Finalize(game);
	return 0;
}
