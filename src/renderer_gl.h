#ifndef _RENDERER_GL_H
#define _RENDERER_GL_H
#include "config.h"
#include "renderer.h"

#define BUFFER_CAPACITY 256

typedef float mat4[16];

struct Renderer {
	u32 vbo, program, spritesheet;
	s32 u_transform;
	mat4 transform;

	int img_width;

	float width, height;

	s32 water_mark;
	s32 buffer_length;
	float quad_buffer[BUFFER_CAPACITY * 4 * 4];
};

void RendererInit(Renderer *);
void RendererResize(Renderer *renderer, int, int);
void RendererEnableDebugLogs(void);
void RendererFlush(Renderer *);
void LoadShaders(Renderer *);

#endif
