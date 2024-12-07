#ifndef _TYPES_H
#define _TYPES_H
#include "config.h"

// Null terminated strings were a mistake
typedef struct str {
	size length;
	unsigned char *data;
} str;

#define str(s) ((str){lengthof(s), (unsigned char *)(s)})

typedef struct {
	float x, y;
} vec2;

#define vec2(x, y) ((vec2){(x), (y)})

static inline vec2 vec2_add(vec2 a, vec2 b)
{
	return (vec2){a.x + b.x, a.y + b.y};
}

static inline vec2 vec2_mulf(vec2 v, float f)
{
	return (vec2){v.x * f, v.y * f};
}

static inline vec2 vec2_divf(vec2 v, float f)
{
	return (vec2){v.x / f, v.y / f};
}

static inline float vec2_dot(vec2 a, vec2 b)
{
	return a.x*b.x + a.y*b.y;
}

#endif
