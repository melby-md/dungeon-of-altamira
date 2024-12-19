#ifndef _RENDERER_GL_H
#define _RENDERER_GL_H
#include "renderer.h"

#define SPRITE_BUFFER_CAPACITY 256

typedef float mat4[16];

typedef struct {
	float pos[2];
	float uv[2];
} QuadVertex;

typedef QuadVertex Quad[4];

struct Renderer {
	u32 framebuffer, sprite_vbo, sprite_vao, static_tiles_vao,
	    static_tiles_vbo;
	s32 u_transform;
	mat4 transform;

	int width, height, top, left, right, bottom;

	s32 static_tiles_length;
	s32 sprite_buffer_length;
	Quad sprite_buffer[SPRITE_BUFFER_CAPACITY];
};

void RendererInit(Renderer *, Arena);
void RendererResize(Renderer *renderer, int, int);
void RendererEnableDebugLogs(void);

#endif
