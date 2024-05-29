#ifndef _ENTITY_H
#define _ENTITY_H
#include <stdbool.h>

#include "config.h"
#include "arena.h"
#include "gfx.h"

typedef struct Entity Entity;

typedef struct {
	float x0, y0;
	float x1, y1;
} Lerp;

typedef enum {
	PLAYER,
	MONSTER
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
	float x, y;
	enum {
		STANDING,
		MOVING
	} animation;
	Lerp lerp; } Sprite;

struct Entity {
	bool used;
	string name;
	int x, y;
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

enum { // Tile sprites
	FLOOR,
	WALL
};

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

struct GameState {
	PlayerData player;

	int w, h;
	Tile *tiles;

	int entityCount;
	Entity entities[512];

	Uint32 lastUpdate;
};

void DungeonCreate(Arena *, GameState *, int, int);
Entity *EntityCreate(GameState *, Entity *);
void EntityDestroy(GameState *, Entity *);
//Action GetEntityAction(Dungeon *, Entity *);

#endif
