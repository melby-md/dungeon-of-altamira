#include "assets.h"
#include "platform.h"

char *AssetFileName[] = {
#define X(id, file)  "assets/" file,
	ASSETS
#undef X
};

str AssetRead(int asset)
{
	return ReadEntireFile(AssetFileName[asset]);
}
