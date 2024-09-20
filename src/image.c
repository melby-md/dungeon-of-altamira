#include "config.h"
#include "SDL.h"

#define STB_IMAGE_IMPLEMENTATION

#define STBI_MALLOC(x)     SDL_malloc(x)
#define STBI_REALLOC(x, y) SDL_realloc(x, y)
#define STBI_FREE(x)       SDL_free(x)

#define STBI_ASSERT(x) Assert(x)

#define STBI_NO_STDIO
#define STBI_ONLY_PNG

#include "../third-party/stb_image.h"
