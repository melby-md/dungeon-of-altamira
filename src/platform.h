#ifndef _PLATFORM_H
#define _PLATFORM_H
#include "config.h"
#include "types.h"

typedef struct Controls {
	unsigned up    : 1;
	unsigned down  : 1;
	unsigned left  : 1;
	unsigned right : 1;
} Controls;

void  RequestExit(void);
void  Exit(int);
str   ReadEntireFile(const char *);
//void WriteText(Graphics *, str, int, int);
//void UpdateCamera(Graphics *, vec2);
#endif
