#ifndef _TYPES_H
#define _TYPES_H
#include "config.h"

// Null terminated strings were a mistake
typedef struct str {
	size length;
	unsigned char *data;
} str;

#define str(s) ((str){lengthof(s), (unsigned char *)(s)})

typedef float vec2[2];

#define vec2(x, y) ((vec2){(x), (y)})

static inline void vec2_add(vec2 out, const vec2 a, const vec2 b)
{
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
}

static inline void vec2_sub(vec2 out, const vec2 a, const vec2 b)
{
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
}

static inline void vec2_mulf(vec2 out, const vec2 v, float f)
{
	out[0] = v[0] * f;
	out[1] = v[1] * f;
}

static inline void vec2_divf(vec2 out, const vec2 v, float f)
{
	out[0] = v[0] / f;
	out[1] = v[1] / f;
}

static inline float vec2_dot(const vec2 a, const vec2 b)
{
	return a[0]*b[0] + a[1]*b[1];
}

#endif
