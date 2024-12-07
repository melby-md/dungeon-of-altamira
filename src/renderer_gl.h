#ifndef _RENDERER_GL_H
#define _RENDERER_GL_H
#include "config.h"
#include "renderer.h"

#define QUAD_BUFFER_CAPACITY 256

typedef float mat4[16];

typedef struct {
	float pos[2];
	float uv[2];
} QuadVertex;

struct Renderer {
	u32 program;
	s32 u_transform;
	mat4 transform;

	s32 quad_buffer_length;
	QuadVertex quad_buffer[QUAD_BUFFER_CAPACITY * 4];
};

void RendererInit(Renderer *);
void RendererResize(Renderer *renderer, int, int);
void RendererEnableDebugLogs(void);
void RendererFlush(Renderer *);
void LoadShaders(Renderer *);
void ReloadShaders(Renderer *);

#endif
