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
};

enum sprites {
	CHAO,
	PAREDE,
	PESSOA,
	ZUMBI
};

enum assets {
	ASSET_SPRITE_VERTEX_SHADER,
	ASSET_SPRITE_FRAGMENT_SHADER,

	ASSET_SPRITESHEET
};

const char *AssetFileName[] = {
	[ASSET_SPRITE_VERTEX_SHADER] = "assets/main.vert",
	[ASSET_SPRITE_FRAGMENT_SHADER] = "assets/main.frag",
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
		vec2_mulf(movement, movement, elapsedTime * 200.0f);

		vec2_add(state->pos, state->pos, movement);
	}
}

void Render(GameState *state, Renderer *renderer)
{

	vec2 camera;
	vec2_add(camera, state->pos, vec2(50.0f, 50.0f));
	CameraMove(renderer, camera);
	RendererClear();

	DrawQuad(renderer, vec2(200.0f, 200.0f), CHAO);
	DrawQuad(renderer, state->pos, ZUMBI);
	DrawQuad(renderer, vec2(100.0f, 100.0f), PAREDE);
}

GameState *InitGame(u8 *memory, size memorySize, Renderer *renderer)
{
	Arena arena;
	ArenaInit(&arena, memory, memorySize);
	GameState *state = Alloc(&arena, GameState);

	state->pos[0] = 0.0f;
	state->pos[1] = 0.0f;
	state->arena = arena;

	RendererSetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	CameraResize(renderer, 512.0f, 512.0f);

	return state;
}
