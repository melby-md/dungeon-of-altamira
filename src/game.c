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
	GROUND,
	WALL,
	PERSON,
	ZOMBIE
};

AABB translateBox(vec2 pos, AABB box)
{
	return (AABB){
		vec2_add(box.min, pos),
		vec2_add(box.max, pos)
	};
}

int realToTilePos(float x)
{
	return (int)x - (x < 0.f ? 1 : 0);
}

bool colisionAgainstTileMap(Entity *e, GameState *state)
{
	AABB box = translateBox(e->pos, e->box);

	int x0 = realToTilePos(box.min.x);
	int y0 = realToTilePos(box.min.y);
	int x1 = realToTilePos(box.max.x);
	int y1 = realToTilePos(box.max.y);

	for (int x = x0; x <= x1; x++)
		for (int y = y0; y <= y1; y++) {
			if (x < 0 || x >= state->dungeon_width)
				return true;

			if (y < 0 || y >= state->dungeon_height)
				return true;

			if (state->dungeon[y*state->dungeon_height + x] == WALL)
				return true;
		}

	return false;
}

void Update(GameState *state, Controls controls, float dt)
{
	// TODO: Better collision
	if (controls.direction.x != 0.f || controls.direction.y != 0.f) {
		vec2 movement = vec2_mulf(controls.direction, dt * 1.8f);

		vec2 old = state->player.pos;
		state->player.pos.x += movement.x;
		if (colisionAgainstTileMap(&state->player, state))
			state->player.pos = old;

		old = state->player.pos;
		state->player.pos.y += movement.y;
		if (colisionAgainstTileMap(&state->player, state))
			state->player.pos = old;
	}
}

void Render(GameState *state, Renderer *renderer)
{
	BeginCamera(renderer, state->player.pos);
	DrawSprite(renderer, state->player.pos, ZOMBIE);
	EndCamera(renderer);
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

	int width = 20, height = 20;
	state->dungeon = AllocArray(arena, u8, sizeof(u8) * width * height);
	memset(state->dungeon, GROUND, sizeof(u8) * width * height);
	state->dungeon_width = width;
	state->dungeon_height = height;

	state->dungeon[45] = WALL;

	BeginStaticTiles(renderer);
	for (int i = 0; i < state->dungeon_height; i++)
		for (int j = 0; j < state->dungeon_width; j++) {
			int id = state->dungeon[i*state->dungeon_width + j];
			PushTile(renderer, vec2((float)j, (float)i), id);
		}
	PushShadow(renderer, vec2(1.f, 1.f), vec2(2.f, 1.f));
	EndStaticTiles(renderer);

	return state;
}
