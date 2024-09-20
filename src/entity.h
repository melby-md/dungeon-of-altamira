#ifndef _ENTITY_H
#define _ENTITY_H
#include <stdbool.h>

#include "config.h"
#include "arena.h"
#include "dungeon.h"

typedef struct Entity Entity;

typedef enum {
	PLAYER,
	MONSTER,
	FLOOR,
	WALL
} SpriteId;

/*
typedef struct {
	ActionType type;
	union {
		struct { // Movement
			int x, y;
		};
		struct { // Atack
			Entity *target;
		};
	};
} Action;
*/

typedef struct {
	SpriteId id;
	vec2 pos;
	enum {
		STANDING,
		MOVING
	} animation;
	vec4 lerp;
} Sprite;

struct Entity {
	bool used;
	str name;
	vec2i pos;
	int hp;
	int maxHp;
	Sprite sprite;
};

/*
typedef enum {
	NOOP,
	MOVE,
	ATACK
} ActionType;
*/

typedef struct {
	int money;
	Entity *entity;
	//Action nextAction;
} PlayerData;

enum { // Tile flags
	COLISION = 1 << 0,
	FOV	 = 1 << 1,
	OPAQUE   = 1 << 2,
	ITEM     = 1 << 3
};

typedef struct {
	Uint8 sprite;
	Uint8 flags;
} Tile;

typedef struct Dungeon {
	PlayerData player;

	int w, h;
	Tile *tiles;

	int entityCount;
	Entity entities[512];

	s32 lastUpdate;
} Dungeon;

void DungeonCreate(Arena *, GameState *, int, int);
Entity *EntityCreate(GameState *, Entity *);
void EntityDestroy(GameState *, Entity *);
//Action GetEntityAction(Dungeon *, Entity *);

#endif
