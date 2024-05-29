#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "dungeon.h"

static const char *GAME_LIBRARY = "./dungeon.so";

typedef struct {
	void *handle;
	ino_t id;
	GameApi api;
	GameState *state;
	Platform *platform;
} Game;

const char *__asan_default_options(void)  { return "abort_on_error=1:halt_on_error=1:detect_leaks=0"; }
const char *__ubsan_default_options(void) { return "abort_on_error=1:halt_on_error=1"; }

static void
gameLoad(Game *game)
{
	struct stat attr;
	if ((stat(GAME_LIBRARY, &attr) == 0) && (game->id != attr.st_ino)) {
		if (game->handle) {
			dlclose(game->handle);
		}
		void *handle = dlopen(GAME_LIBRARY, RTLD_NOW);
		if (handle) {
			game->handle = handle;
			game->id = attr.st_ino;
			const GameApi *api = dlsym(game->handle, "GAME_API");
			if (api != NULL) {
				game->api = *api;
				if (game->platform == NULL) {
					game->platform = game->api.init_platform();
					if (game->platform == NULL)
						exit(EXIT_FAILURE);
				}
				if (game->state == NULL)
					game->state = game->api.init_game(game->platform);
			} else {
				dlclose(game->handle);
				game->handle = NULL;
				game->id = 0;
			}
		} else {
			game->handle = NULL;
			game->id = 0;
		}
	}
}

static void
gameUnload(Game *game)
{
	if (game->handle) {
		game->api.finalize(game->platform);
		game->state = NULL;
		dlclose(game->handle);
		game->handle = NULL;
		game->id = 0;
	}
}

int
main(void)
{
	Game game = {0};
	for (;;) {
		gameLoad(&game);
		if (game.handle) {
			int stat = game.api.step(game.platform, game.state);
			switch (stat) {
			case GAME_QUIT:
				gameUnload(&game);
				return EXIT_SUCCESS;
			case GAME_RELOAD:
				Log("reloading\n");
				game.state = game.api.init_game(game.platform);
			}
		} else {
			usleep(10000);
		}
	}

	Assert(0 && "This should be unreachable");
	return 0;
}
