#include "assets.h"
#include "common.h"
#include "game.h"
#include "renderer.h"
#include "platform.h"
#include "arena.h"

typedef struct AABB {
	vec2 min, max;
} AABB;

typedef struct Entity {
	vec2 pos;
	AABB box;
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

AABB translateBox(vec2 pos, AABB box)
{
	return (AABB){
		vec2_add(box.min, pos),
		vec2_add(box.max, pos)
	};
}

bool getTileMapColision(vec2 pos, GameState *state)
{
	if (pos.x < 0.f || pos.y < 0.f)
		return true;

	int x = (int)pos.x;
	int y = (int)pos.y;
	
	if (x >= state->dungeon_width)
		return true;

	if (y >= state->dungeon_height)
		return true;

	return state->dungeon[y*state->dungeon_height + x] == PAREDE;
}

bool colisionAgainstTileMap(Entity *e, GameState *state)
{
	AABB box = translateBox(e->pos, e->box);

	vec2 points[4] = {
		{box.min.x, box.min.y},
		{box.max.x, box.min.y},
		{box.max.x, box.max.y},
		{box.min.x, box.max.y}
	};

	for (int i = 0; i < countof(points); i++) {
		if (getTileMapColision(points[i], state))
			return true;
	}

	return false;
}

void Update(GameState *state, Controls controls, float dt)
{
	if (controls.direction.x != 0.f || controls.direction.y != 0.f) {
		vec2 movement = vec2_mulf(controls.direction, dt * .8f);
		vec2 old = state->player.pos;
		state->player.pos = vec2_add(state->player.pos, movement);
		if (colisionAgainstTileMap(&state->player, state))
			state->player.pos = old;
	}
}

void Render(GameState *state, Renderer *renderer)
{
	CameraMove(renderer, state->player.pos);

	DrawSprite(renderer, state->player.pos, ZUMBI);
}

GameState *InitGame(Arena *arena, Renderer *renderer)
{
	GameState *state = Alloc(arena, GameState);
	state->player.pos = vec2(0.f, 0.f);
	state->player.box = (AABB){
		{.18f, 0.f},
		{.81f, 1.f}
	};
	state->arena = *arena;

	int width = 2, height = 20;
	state->dungeon = AllocArray(arena, u8, sizeof(u8) * width * height);
	memset(state->dungeon, CHAO, sizeof(u8) * width * height);
	state->dungeon_width = width;
	state->dungeon_height = height;

	BegStaticTiles(renderer);
	for (int i = 0; i < state->dungeon_height; i++)
		for (int j = 0; j < state->dungeon_width; j++) {
			int id = state->dungeon[i*state->dungeon_width + j];
			PushTile(renderer, vec2((float)j, (float)i), id);
		}
	EndStaticTiles(renderer);

	return state;
}
