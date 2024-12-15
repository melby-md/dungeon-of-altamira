// Odds and ends...
#ifndef _CONFIG_H
#define _CONFIG_H
#include <math.h>
#include <stddef.h> // ptrdiff_t
#include <stdint.h> // *int*_t
#include <string.h> // memset, memcpy, strlen, etc...

#include "SDL.h"

// macros

#define countof(x) (sizeof(x)/sizeof((x)[0]))

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#ifdef RELEASE
#  define ErrorStr(str) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s", str)
#  define Error(...) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, __VA_ARGS__)
#  define Log(...)
#  define Assert(c)
#else
#  define DEBUG
#  define xstr(x) STR(x)
#  define STR(x) #x
#  define ErrorStr(str) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, __FILE__ ":" xstr(__LINE__) ": %s", str)
#  define Error(...) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, __FILE__ ":" xstr(__LINE__) ": " __VA_ARGS__)
#  define Log(...) SDL_Log(__FILE__ ":" xstr(__LINE__) ": " __VA_ARGS__)
#  if __GNUC__
#    define Assert(c) if (!(c)) __builtin_trap()
#  elif _MSC_VER
#    define Assert(c) if (!(c)) __debugbreak()
#  else
#    define Assert(c) if (!(c)) *(volatile int *)0 = 0
#  endif
#endif

#define NORETURN _Noreturn

// types

typedef uintptr_t uptr;
typedef ptrdiff_t size;

typedef uint64_t  u64;
typedef uint32_t  u32;
typedef uint16_t  u16;
typedef uint8_t   u8;

typedef int64_t   s64;
typedef int32_t   s32;
typedef int16_t   s16;
typedef int8_t    s8;

typedef struct str {
	size length;
	unsigned char *data;
} str;

#define str_length(x) (countof(x)-1)
#define str(s) ((str){str_length(s)-1, (unsigned char *)(s)})

typedef struct {
	float x, y;
} vec2;

#define vec2(x, y) ((vec2){(x), (y)})

static inline vec2 vec2_add(vec2 a, vec2 b)
{
	return (vec2){a.x + b.x, a.y + b.y};
}

static inline vec2 vec2_sub(vec2 a, vec2 b)
{
	return (vec2){a.x - b.x, a.y - b.y};
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
