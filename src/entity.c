#include "entity.h"

Entity *
EntityCreate(GameState *dungeon, Entity *data)
{
	Uint8 *tileFlags = &dungeon->tiles[data->pos.x + data->pos.y*dungeon->w].flags;
	if (*tileFlags & COLISION)
		return NULL;
	*tileFlags |= COLISION;

	Entity *e = &dungeon->entities[dungeon->entityCount++];

	*e = *data;

	e->used = true;
	e->sprite.x = (float)(e->pos.x * 16);
	e->sprite.y = (float)(e->pos.y * 16);

	return e;
}

void
DungeonCreate(Arena *arena, GameState *dungeon, int w, int h)
{
	dungeon->w = w;
	dungeon->h = h;
	int dimensions = w * h;
	dungeon->tiles = AllocArray(arena, Tile, dimensions);
	for (int i = 0; i < dimensions; i++)
		dungeon->tiles[i] = (Tile){FLOOR, FOV};

	dungeon->entityCount = 0;
	dungeon->player.money = 0;
	dungeon->player.entity = EntityCreate(dungeon, &(Entity){
		.name = StringDup(arena, str("João")),
		.hp = 10,
		.maxHp = 10,
		.sprite = {.id = PLAYER, .animation = STANDING},
	});
}

void
EntityDestroy(GameState *dungeon, Entity *e)
{
	e->used = false;
	dungeon->entityCount--;
}

int
EntityMove(GameState *dungeon, Entity *e, int x, int y)
{
	if (x < 0 || y < 0 || x >= dungeon->w || y >= dungeon->h)
		return false;

	int p = x + y * dungeon->w;
	if (dungeon->tiles[p].flags & COLISION)
		return false;

	dungeon->tiles[p].flags |= COLISION;

	e->x = x;
	e->y = y;

	return true;
}
/*
Action
GetEntityAction(GameState *dungeon, Entity *e)
{
	(void)dungeon;
	(void)e;

	return (Action){NOOP};
}
*/
