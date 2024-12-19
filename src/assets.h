#ifndef _ASSETS_H
#define _ASSETS_H
#include "common.h"

#define ASSETS \
	X(QUAD_VERTEX_SHADER, "assets/shaders/quad.vert") \
	X(QUAD_FRAGMENT_SHADER, "assets/shaders/quad.frag") \
	X(SPRITESHEET, "assets/spritesheet.png")

enum assets {
#define X(id, file) ASSET_##id,
	ASSETS
#undef X
	assets_length
};


str AssetRead(int);

#endif
