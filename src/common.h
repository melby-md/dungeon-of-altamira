// Odds and ends...
#ifndef _CONFIG_H
#define _CONFIG_H
#include <math.h>
#include <stddef.h> // ptrdiff_t
#include <stdint.h> // *int*_t
#include <string.h> // memset, memcpy, strlen, etc...
#include <stdbool.h>

#include "SDL.h"

// macros

#define countof(x) (size)(sizeof(x)/sizeof((x)[0]))

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#ifdef RELEASE
#  define ErrorStr(str) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s", str)
#  define Error(...) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, __VA_ARGS__)
#  define Log(...)
#  define DebugBreak()
#else
#  define DEBUG
#  define xstr(x) STR(x)
#  define STR(x) #x
#  define ErrorStr(str) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, __FILE__ ":" xstr(__LINE__) ": %s", str)
#  define Error(...) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, __FILE__ ":" xstr(__LINE__) ": " __VA_ARGS__)
#  define Log(...) SDL_Log(__FILE__ ":" xstr(__LINE__) ": " __VA_ARGS__)
#  if __GNUC__
#    define Break() __builtin_trap()
#  elif _MSC_VER
#    define Break() __debugbreak()
#  else
#    define Break() *(volatile int *)0 = 0
#  endif
#endif

#define Assert(c) if (!(c)) Break()

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

typedef struct Str {
	size length;
	unsigned char *data;
} Str;

#define StrLength(x) (countof(x)-1)
#define Str(s) ((str){StrLength(s)-1, (unsigned char *)(s)})

typedef struct Vec3 {
	float x, y, z;
} Vec3;

typedef struct Vec2 {
	float x, y;
} Vec2;

static inline Vec2 Vec2Add(Vec2 a, Vec2 b)
{
	return (Vec2){a.x + b.x, a.y + b.y};
}

static inline Vec2 Vec2Sub(Vec2 a, Vec2 b)
{
	return (Vec2){a.x - b.x, a.y - b.y};
}

static inline Vec2 Vec2Scale(Vec2 v, float f)
{
	return (Vec2){v.x * f, v.y * f};
}

static inline Vec2 Vec2Normalize(Vec2 v)
{
	float length = sqrtf(v.x * v.x + v.y * v.y);
	return (Vec2){v.x / length, v.y / length};
}

#endif
