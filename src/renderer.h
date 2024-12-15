#ifndef _RENDERER_H
#define _RENDERER_H
#include "common.h"

typedef struct Renderer Renderer;

void DrawQuad(Renderer *, vec2, int);
void CameraMove(Renderer *, vec2);

#endif
