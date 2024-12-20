#ifndef _RENDERER_H
#define _RENDERER_H
#include "arena.h"
#include "common.h"

typedef struct Renderer Renderer;

// Used for sprites that change every frame (ex.: player and enemies)
void BeginCamera(Renderer *, vec2);
void DrawSprite(Renderer *, vec2, int);
void EndCamera(Renderer *);

// Used for sprites that don't change frequently (tiles) (ex.: background)
void BeginStaticTiles(Renderer *);
void PushTile(Renderer *, vec2, int);
void EndStaticTiles(Renderer *);

void BeginShadows(Renderer *);
void PushShadow(Renderer *, vec2, vec2);
void EndShadows(Renderer *);

#endif
