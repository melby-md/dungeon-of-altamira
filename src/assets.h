#ifndef _ASSETS_H
#define _ASSETS_H
#include "common.h"

#define ASSETS \
	X(QUAD_VERTEX_SHADER, "shaders/quad.vert") \
	X(QUAD_FRAGMENT_SHADER, "shaders/quad.frag") \
	X(SHADOW_VERTEX_SHADER, "shaders/shadow.vert") \
	X(SHADOW_FRAGMENT_SHADER, "shaders/shadow.frag") \
	X(SPRITESHEET, "spritesheet.png")

enum assets {
#define X(id, file) ASSET_##id,
	ASSETS
#undef X
	assets_length
};


str AssetRead(int);

#endif
