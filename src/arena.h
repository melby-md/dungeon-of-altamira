#ifndef _ARENA_H
#define _ARENA_H
#include "types.h"

#define AllocArray(arena, type, n) (type *)alloc(arena, sizeof(type) * n, _Alignof(type))
#define Alloc(arena, type) AllocArray(arena, type, 1)

typedef struct Arena {
	char *beg;
	char *end;
} Arena;

Arena AllocTempArena(Arena *);
str StringDup(Arena *, str);
void ArenaInit(Arena *, char *, size);

void *alloc(Arena *, size, size);

#endif
