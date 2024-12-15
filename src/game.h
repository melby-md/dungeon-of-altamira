#ifndef _GAME_H
#define _GAME_H
#include "arena.h"
#include "common.h"
#include "platform.h"
#include "renderer.h"

typedef struct GameState GameState;

GameState *InitGame(Arena *);
void Update(GameState *, Controls, float);
void Render(GameState *, Renderer *);

#endif
