#ifndef _PLATFORM_H
#define _PLATFORM_H
#include "common.h"

typedef struct Controls {
	Vec2 direction;
} Controls;

void RequestExit(void);
void Panic(const char *);
Str  ReadEntireFile(const char *);
#endif
