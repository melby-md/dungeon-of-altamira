#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "config.h"
#include "hotreload.h"

typedef struct Code {
	void *handle;
	ino_t id;
	GameApi *api;
	Game *state;
} Code;

const char *__asan_default_options(void)  { return "abort_on_error=1:halt_on_error=1:detect_leaks=0"; }
const char *__ubsan_default_options(void) { return "abort_on_error=1:halt_on_error=1"; }

static void
gameLoad(Code *game)
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
			game->api = dlsym(game->handle, "GAME_API");
			if (game->api != NULL) {
				if (game->state == NULL) {
					game->state = game->api->init();
					if (game->state == NULL)
						exit(1);
				}
				Log("Reloaded!");
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
gameUnload(Code *game)
{
	if (game->handle) {
		game->api->finalize(game->state);
		game->state = NULL;
		dlclose(game->handle);
		game->handle = NULL;
		game->id = 0;
	}
}

int
main(void)
{
	Code game = {0};
	for (;;) {
		gameLoad(&game);
		if (game.handle) {
			game.api->loop(game.state);
			if (!game.api->running)
				break;
		} else {
			usleep(10000);
		}
	}

	gameUnload(&game);

	return 0;
}
