#ifndef _ASSETS_H
#define _ASSETS_H
#include "common.h"

#define ASSETS \
	X(VERTEX_SHADER, "assets/main.vert") \
	X(FRAGMENT_SHADER, "assets/main.frag") \
	X(SPRITESHEET, "assets/spritesheet.png")

enum assets {
#define X(id, file) ASSET_##id,
	ASSETS
#undef X
	assets_length
};


str AssetRead(int);

#endif
