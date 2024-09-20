#ifndef _GAME_H
#define _GAME_H
#include "platform.h"
#include "config.h"
#include "renderer.h"

typedef struct GameState GameState;

GameState *InitGame(u8 *, size, Renderer *renderer);
void Update(GameState *, Controls, float);
void Render(GameState *, Renderer *);

#endif
