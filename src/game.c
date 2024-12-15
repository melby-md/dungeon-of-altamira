#include "assets.h"
#include "common.h"
#include "game.h"
#include "renderer.h"
#include "platform.h"
#include "arena.h"

typedef struct Entity {
	vec2 pos;
} Entity;

struct GameState {
	Entity player;
	Arena arena;

	u8 *dungeon;
	int dungeon_height, dungeon_width;
};

enum sprites {
	CHAO,
	PAREDE,
	PESSOA,
	ZUMBI
};

void Update(GameState *state, Controls controls, float dt)
{
	if (controls.direction.x != 0.f || controls.direction.y != 0.f) {
		vec2 movement = vec2_mulf(controls.direction, dt * 18.f);
		state->player.pos = vec2_add(state->player.pos, movement);
	}
}

void Render(GameState *state, Renderer *renderer)
{
	vec2 camera = vec2_add(state->player.pos, vec2(8.f, 8.f));
	CameraMove(renderer, camera);

	for (int i = 0; i < state->dungeon_height; i++)
		for (int j = 0; j < state->dungeon_width; j++) {
			int id = state->dungeon[i*state->dungeon_width + j];
			DrawQuad(renderer, vec2((float)(j*16), (float)(i*16)), id);
		}

	DrawQuad(renderer, state->player.pos, ZUMBI);
}

GameState *InitGame(Arena *arena)
{
	GameState *state = Alloc(arena, GameState);
	state->player.pos = vec2(0.f, 0.f);
	state->arena = *arena;

	int width = 2, height = 20;
	state->dungeon = AllocArray(arena, u8, sizeof(u8) * width * height);
	memset(state->dungeon, CHAO, sizeof(u8) * width * height);
	state->dungeon_width = width;
	state->dungeon_height = height;

	return state;
}
