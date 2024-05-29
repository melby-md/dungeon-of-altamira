// Odds and ends...
#ifndef _CONFIG_H
#define _CONFIG_H
#include <assert.h>  // static_assert
#include <stddef.h>  // ptrdiff_t
#include <stdint.h>  // uintptr_t

#ifdef RELEASE
#  define ErrorStr(str) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "%s", str)
#  define Error(...) SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, __VA_ARGS__)
#  define Log(...)
#  define Assert(cond)
#else
#  define DEBUG
#  define xstr(x) str(x)
#  define str(x) #x
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

#include "SDL.h"

typedef uintptr_t uptr;
typedef ptrdiff_t size;

typedef uint64_t  u64;
typedef uint32_t  u32;
typedef uint16_t  u16;
typedef uint8_t   u8;

typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

#endif
