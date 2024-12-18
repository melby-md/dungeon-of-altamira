#include "hotreload.h"
#include "platform.h"

int main(int argc, char *argv[]) {
	Game *game = GAME_API.init();
	while (GAME_API.running) {
		GAME_API.loop(game);
	}

	return 0;
}
