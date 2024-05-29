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

string
StringDup(Arena *a, string src)
{
	string str;

	str.length = src.length;
	str.data = AllocArray(a, unsigned char, str.length);

	memcpy(str.data, src.data, str.length);

	return str;
}
