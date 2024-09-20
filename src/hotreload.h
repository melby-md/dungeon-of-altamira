#ifndef _HOTRELOAD_H
#define _HOTRELOAD_H
#include <stdbool.h>

typedef struct Game Game;

typedef struct GameApi {
	Game *(*init)(void);
	void  (*loop)(Game *);
	void  (*finalize)(Game *);
	bool  running;
} GameApi;

extern GameApi GAME_API;

#endif
