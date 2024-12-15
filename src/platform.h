#ifndef _PLATFORM_H
#define _PLATFORM_H
#include "common.h"

typedef struct Controls {
	vec2 direction;
} Controls;

void  RequestExit(void);
void  Exit(int);
str   ReadEntireFile(const char *);
//void WriteText(Graphics *, str, int, int);
//void UpdateCamera(Graphics *, vec2);
#endif
