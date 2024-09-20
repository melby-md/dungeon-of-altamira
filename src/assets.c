#include "assets.h"
#include "platform.h"


str AssetRead(int asset)
{
	return ReadEntireFile(AssetFileName[asset]);
}
