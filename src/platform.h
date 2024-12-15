#ifndef _PLATFORM_H
#define _PLATFORM_H
#include "common.h"

typedef struct Controls {
	vec2 direction;
} Controls;

void RequestExit(void);
void Panic(const char *);
str  ReadEntireFile(const char *);
#endif
