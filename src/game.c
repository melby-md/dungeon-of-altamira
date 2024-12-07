#include <math.h>

#include "assets.h"
#include "config.h"
#include "game.h"
#include "renderer.h"
#include "platform.h"
#include "arena.h"

struct GameState {
	vec2 pos;
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

enum assets {
	ASSET_SPRITESHEET
};

const char *AssetFileName[] = {
	[ASSET_SPRITESHEET] = "assets/spritesheet.png"
};

void Update(GameState *state, Controls controls, float elapsedTime)
{
	vec2 movement = {
		(float)(controls.right - controls.left),
		(float)(controls.down - controls.up)
	};

	if (movement[0] != 0.0f || movement[1] != 0.0f) {

		vec2_divf(movement, movement, SDL_sqrtf(vec2_dot(movement, movement)));
		vec2_mulf(movement, movement, elapsedTime * 2.0f);

		vec2_add(state->pos, state->pos, movement);
	}
}

void Render(GameState *state, Renderer *renderer)
{

	vec2 camera;
	vec2_add(camera, state->pos, vec2(0.5f, 0.5f));
	CameraMove(renderer, camera);
	RendererClear();

	for (int i = 0; i < state->dungeon_height; i++)
		for (int j = 0; j < state->dungeon_width; j++)
			DrawQuad(renderer, vec2((float)j, (float)i), 1.0f, state->dungeon[j + i*state->dungeon_height]);

	DrawQuad(renderer, state->pos, 1.0f, ZUMBI);
}

GameState *InitGame(char *memory, size memorySize, Renderer *renderer)
{
	Arena arena;
	ArenaInit(&arena, memory, memorySize);
	GameState *state = Alloc(&arena, GameState);

	state->pos[0] = 0.0f;
	state->pos[1] = 0.0f;
	state->arena = arena;

	RendererSetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	CameraResize(renderer, 6.0f, 6.0f);
	SpritesheetLoad(renderer, ASSET_SPRITESHEET);

	int width = 2, height = 20;
	state->dungeon = AllocArray(&arena, u8, sizeof(u8) * width * height);
	memset(state->dungeon, CHAO, sizeof(u8) * width * height);
	state->dungeon_width = width;
	state->dungeon_height = height;

	return state;
}
