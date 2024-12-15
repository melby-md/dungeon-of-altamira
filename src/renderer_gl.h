#ifndef _RENDERER_GL_H
#define _RENDERER_GL_H
#include "renderer.h"

#define SPRITE_BUFFER_CAPACITY 256

typedef float mat4[16];

typedef struct {
	float pos[2];
	float uv[2];
} QuadVertex;

struct Renderer {
	u32 shader, framebuffer, framebuffer_texture, sprite_vbo, screen_vbo, screen_vao, sprite_vao, spritesheet;
	s32 u_transform;
	mat4 transform;
	mat4 identity;

	int width, height;

	s32 sprite_buffer_length;
	QuadVertex sprite_buffer[SPRITE_BUFFER_CAPACITY * 4];
};

void RendererInit(Renderer *, Arena);
void RendererResize(Renderer *renderer, int, int);
void RendererEnableDebugLogs(void);
void RendererBegin(Renderer *);
void RendererEnd(Renderer *);

#endif
