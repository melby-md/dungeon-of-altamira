#ifndef _RENDERER_H
#define _RENDERER_H
#include "arena.h"
#include "common.h"

typedef struct Renderer Renderer;

// Used for sprites that change every frame (ex.: player and enemies)
void BeginCamera(Renderer *, Vec2);
void DrawSprite(Renderer *, Vec2, int);
void EndCamera(Renderer *);

// Used for elements that don't change frequently (background tiles and shadows)
void BeginStaticTiles(Renderer *);
void PushTile(Renderer *, Vec2, int);
void PushShadow(Renderer *, Vec2, Vec2);
void EndStaticTiles(Renderer *);

#endif
