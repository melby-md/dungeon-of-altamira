#ifndef _RENDERER_H
#define _RENDERER_H
#include "types.h"

typedef struct Renderer Renderer;

void RendererSetClearColor(float, float, float, float);
void RendererClear(void);
void DrawQuad(Renderer *, const vec2, int);
void SpritesheetLoad(Renderer *, int);

void CameraResize(Renderer *, float, float);
void CameraMove(Renderer *, const vec2);

#endif
