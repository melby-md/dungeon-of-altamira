#ifndef _ARENA_H
#define _ARENA_H
#include "config.h"

#define countof(x) (sizeof(x)/sizeof((x)[0]))
#define lengthof(x) (countof(x) - 1)

#define AllocArray(arena, type, n) (type *)alloc(arena, sizeof(type) * n, _Alignof(type))
#define Alloc(arena, type) AllocArray(arena, type, 1)
#define s(str) {lengthof(str), (unsigned char *)str}
#define string(str) (string) s(str)

typedef struct Arena {
	u8 *beg;
	u8 *end;
} Arena;

// Null terminated strings were a mistake
typedef struct string {
	size length;
	unsigned char *data;
} string;

Arena AllocTempArena(Arena *);
string StringDup(Arena *, string);

#if __GNUC__
__attribute((malloc, alloc_size(2)))
#endif
void *alloc(Arena *, size, size);

#endif
