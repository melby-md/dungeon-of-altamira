#include "image.h"
#include "SDL.h"
#include "common.h"

#define STB_IMAGE_IMPLEMENTATION

#define STBI_MALLOC(x)     SDL_malloc(x)
#define STBI_REALLOC(x, y) SDL_realloc(x, y)
#define STBI_FREE(x)       SDL_free(x)

#define STBI_ASSERT(x) Assert(x)

#define STBI_NO_STDIO
#define STBI_ONLY_PNG

#include "../third-party/stb_image.h"

unsigned char *ImageDecode(const unsigned char *data, int length, int *width, int *height, int channels)
{
	int nr_channels;
	return stbi_load_from_memory(data, length, width, height, &nr_channels, channels);
}

void ImageFree(void *ptr)
{
	stbi_image_free(ptr);
}
