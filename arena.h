#ifndef _ARENA_H
#define _ARENA_H
#include "config.h"

#define AllocArray(arena, type, n) (type *)alloc(arena, sizeof(type) * n, _Alignof(type))
#define Alloc(arena, type) AllocArray(arena, type, 1)

typedef struct Arena {
	u8 *beg;
	u8 *end;
} Arena;

Arena AllocTempArena(Arena *);
str StringDup(Arena *, str);

#if __GNUC__
__attribute((malloc, alloc_size(2)))
#endif
void *alloc(Arena *, size, size);

#endif
