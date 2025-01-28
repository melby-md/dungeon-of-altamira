#include <stdlib.h> // exit
#include <string.h> // memcpy

#include "arena.h"

NORETURN static void oom(void)
{
	Error("Out of memory.");
	exit(70);
}

Arena AllocTempArena(Arena *a)
{
    Arena tmp;
    size cap = (a->end - a->beg) / 2;
    tmp.beg  = AllocArray(a, char, cap);
    tmp.end = tmp.beg + cap;
    return tmp;
}

void *alloc(Arena *a, size length, size align)
{
    size padding = -(uptr)a->beg & (align - 1);
    size available = a->end - a->beg - padding;
    if (length > available)
        oom();

    void *p = a->beg + padding;
    a->beg += padding + length;
    return p;
}

Str StringDup(Arena *a, Str src)
{
	Str dst;

	dst.length = src.length;
	dst.data = AllocArray(a, unsigned char, dst.length);

	memcpy(dst.data, src.data, dst.length);

	return dst;
}

void ArenaInit(Arena *a, char *mem, size length)
{
	a->beg = mem;
	a->end = mem + length;
}
