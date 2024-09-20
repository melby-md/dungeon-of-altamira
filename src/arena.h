#ifndef _ARENA_H
#define _ARENA_H
#include "config.h"
#include "types.h"

#define AllocArray(arena, type, n) (type *)alloc(arena, sizeof(type) * n, _Alignof(type))
#define Alloc(arena, type) AllocArray(arena, type, 1)

typedef struct Arena {
	u8 *beg;
	u8 *end;
} Arena;

Arena AllocTempArena(Arena *);
str StringDup(Arena *, str);
void ArenaInit(Arena *, u8 *, size);

void *alloc(Arena *, size, size);

#endif
