#ifndef _RENDERER_H
#define _RENDERER_H
#include "types.h"

typedef struct Renderer Renderer;

void RendererSetClearColor(float, float, float, float);
void RendererClear(void);
void DrawQuad(Renderer *, vec2, float, int);
void SpritesheetLoad(Renderer *, int);

void CameraResize(Renderer *, float, float);
void CameraMove(Renderer *, vec2);

#endif
