#ifndef DUNGEON_H_
#define DUNGEON_H_
#include "config.h"

enum {
	GAME_OK,
	GAME_QUIT,
	GAME_RELOAD
};

typedef struct Platform Platform;
typedef struct GameState GameState;

typedef struct GameApi {
    Platform  *(*init_platform)(void);
    GameState *(*init_game)(Platform *);
    void (*finalize)(Platform *);
    int  (*step)(Platform *, GameState *);
} GameApi;

extern const GameApi GAME_API;

#endif
