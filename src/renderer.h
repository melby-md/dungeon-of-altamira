#ifndef _RENDERER_H
#define _RENDERER_H
#include "arena.h"
#include "common.h"

typedef struct Renderer Renderer;

// Used for sprites that change every frame (ex.: player and enemies)
void BeginCamera(Renderer *, vec2);
void DrawSprite(Renderer *, vec2, int);
void EndCamera(Renderer *);

// Used for elements that don't change frequently (background tiles and shadows)
void BeginStaticTiles(Renderer *);
void PushTile(Renderer *, vec2, int);
void PushShadow(Renderer *, vec2, vec2);
void EndStaticTiles(Renderer *);

#endif
