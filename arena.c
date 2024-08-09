#include <stdlib.h> // exit
#include <string.h> // memcpy

#include "arena.h"

NORETURN static void
oom(void)
{
	Error("Out of memory.");
	exit(EXIT_FAILURE);
}

Arena
AllocTempArena(Arena *a)
{
    Arena tmp;
    size cap = (a->end - a->beg) / 2;
    tmp.beg  = AllocArray(a, u8, cap);
    tmp.end = tmp.beg + cap;
    return tmp;
}

void *
alloc(Arena *a, size length, size align)
{
    size padding = -(uptr)a->beg & (align - 1);
    size available = a->end - a->beg - padding;
    if (length > available)
        oom();

    void *p = a->beg + padding;
    a->beg += padding + length;
    return p;
}

str
StringDup(Arena *a, str src)
{
	str dst;

	dst.length = src.length;
	dst.data = AllocArray(a, unsigned char, dst.length);

	memcpy(dst.data, src.data, dst.length);

	return dst;
}
