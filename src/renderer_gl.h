#ifndef _RENDERER_GL_H
#define _RENDERER_GL_H
#include "renderer.h"

#define SPRITE_BUFFER_CAPACITY 1024

typedef struct QuadVertex {
	Vec2 pos;
	Vec2 uv;
} QuadVertex;

typedef struct ShadowVertex {
	Vec3 pos;
} ShadowVertex;

typedef QuadVertex Quad[4];
typedef ShadowVertex Shadow[4];

typedef struct UBO {
	Vec2 translate;
	Vec2 scale;
	Vec2 light_pos;
} UBO;

struct Renderer {
	u32 framebuffer, sprite_vbo, sprite_vao, static_tiles_vao,
	    static_tiles_vbo, shadow_vbo, shadow_vao, quad_program, shadow_program;

	UBO ubo;

	int width, height, top, left, right, bottom;

	s32 static_tiles_length;
	s32 sprite_buffer_length;
	Quad sprite_buffer[SPRITE_BUFFER_CAPACITY];

	s32 shadow_buffer_length;
	Shadow shadow_buffer[SPRITE_BUFFER_CAPACITY];
};

void RendererInit(Renderer *, Arena, bool);
void RendererResize(Renderer *, int, int);

void BeginRender(Renderer *);
void EndRender(Renderer *);

#endif
