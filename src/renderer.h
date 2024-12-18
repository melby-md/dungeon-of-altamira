#ifndef _RENDERER_H
#define _RENDERER_H
#include "arena.h"
#include "common.h"

typedef struct Renderer Renderer;

// Used for sprites that change every frame (ex.: player and enemies)
void DrawSprite(Renderer *, vec2, int);

// Used for sprites that don't change frequently (tiles) (ex.: background)
void BegStaticTiles(Renderer *);
void PushTile(Renderer *, vec2, int);
void EndStaticTiles(Renderer *);

void CameraMove(Renderer *, vec2);

#endif
