#include "SDL.h"

#include "config.h"
#include "dungeon.h"
#include "entity.h"
#include "gfx.h"

static const float movTime = 150.0f;

static const int FPS = 60;
static const int targetTime = 1000 / FPS;

static int
Loop(Platform *platform, GameState *dungeon)
{
	PollControls(&platform->graphics);
	Controls controls = platform->graphics.control;

	if (controls.quit)
		return GAME_QUIT;
	if (controls.reload)
		return GAME_RELOAD;

	u32 startTime = SDL_GetTicks();

	// Frame rate capping
	{
		u32 elapsed = startTime - platform->graphics.lastFrame;

		if (elapsed < targetTime) {
			SDL_Delay(targetTime - elapsed);
			startTime = SDL_GetTicks();
		}

		platform->graphics.lastFrame = startTime;
	}

	float elapsedTime = (float)(startTime - dungeon->lastUpdate);

	Entity *player = dungeon->player.entity;

	if (elapsedTime > movTime) {
		dungeon->lastUpdate = startTime;

		Sint8 x = 0, y = 0;
		if (controls.up && !controls.down)
			y = -1;
		else if (!controls.up && controls.down)
			y = 1;

		if (player->y + y < 0 || player->y + y >= dungeon->h)
			y = 0;
		else {
			int p = player->x + (player->y + y) * dungeon->w;
			if (dungeon->tiles[p].flags & COLISION)
				y = 0;
		}

		if (controls.right && !controls.left)
			x = 1;
		else if (!controls.right && controls.left)
			x = -1;

		if (player->x + x < 0 || player->x + x >= dungeon->w)
			x = 0;
		else {
			int p = x + player->x + player->y * dungeon->w;
			if (dungeon->tiles[p].flags & COLISION)
				x = 0;
		}

		int p = x + player->x + (y + player->y) * dungeon->w;
		if (dungeon->tiles[p].flags & COLISION) {
			x = y = 0;
		}

		player->sprite.animation = STANDING;
		player->sprite.lerp.x0 = (float)player->x;
		player->sprite.lerp.y0 = (float)player->y;

		dungeon->tiles[player->x + player->y * dungeon->w].flags &= ~COLISION;

		if (x) {
			player->sprite.animation = MOVING;
			player->x += x;
			player->sprite.lerp.x1 = (float)player->x;
		} else {
			player->sprite.x = (float)player->x;
			player->sprite.lerp.x1 = player->sprite.lerp.x0;
		}

		if (y) {
			player->sprite.animation = MOVING;
			player->y += y;
			player->sprite.lerp.y1 = (float)player->y;
		} else {
			player->sprite.y = (float)player->y;
			player->sprite.lerp.y1 = player->sprite.lerp.y0;
		}

		dungeon->tiles[player->x + player->y * dungeon->w].flags |= COLISION;


	} else {

		if (player->sprite.animation == MOVING) {
			float t = elapsedTime / movTime;
			// TODO: Write a lerp function
			//player->sprite.x = (1.0f-t) * player->sprite.lerp.x0 + t * player->sprite.lerp.x1;
			//player->sprite.y = (1.0f-t) * player->sprite.lerp.y0 + t * player->sprite.lerp.y1;
			player->sprite.x = player->sprite.lerp.x0 + t * (player->sprite.lerp.x1 - player->sprite.lerp.x0);
			player->sprite.y = player->sprite.lerp.y0 + t * (player->sprite.lerp.y1 - player->sprite.lerp.y0);
		}
	}


	Render(platform, dungeon);

	return GAME_OK;
}

static GameState *
Init(Platform *p)
{
	// Hot reloading stuff
	p->arena.beg = p->memory;
	p->graphics.control.reload = 0;

	GameState *dungeon = Alloc(&p->arena, GameState);
	DungeonCreate(&p->arena, dungeon, 70, 70);
	//dungeon->tiles[25] = (Tile){WALL, COLISION | FOV};

	/*EntityCreate(dungeon, &(Entity) {
		.name = AllocString(&p->arena, "Goblin"),
		.x = 7, .y = 7,
		.hp = 10, .maxHp = 10,
		.sprite = {.id = MONSTER, .animation = STANDING},
	});*/

	dungeon->lastUpdate = SDL_GetTicks();
	return dungeon;
}

const GameApi GAME_API = {
	.init_platform = InitPlatform,
	.init_game     = Init,
	.finalize      = Exit,
	.step          = Loop
};
