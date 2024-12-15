#include <stddef.h> // offsetof

#include "arena.h"
#include "assets.h"
#include "common.h"
#include "config.h"
#include "image.h"
#include "platform.h"
#include "renderer.h"
#include "renderer_gl.h"

#ifdef GL_ES
#  include <GLES3/gl3.h>
char shader_header[] = "#version 300 es\n";
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/gl.h>
char shader_header[] = "#version 330 core\n";
#endif

#define LOG_SIZE 1024

static u32 compileShader(int src, GLenum type)
{
	str file = AssetRead(src);

	char *src_arr[] = {
		shader_header,
		(char *)file.data
	};

	s32 length_arr[] = {
		(s32)str_length(shader_header),
		(s32)file.length
	};

	u32 shader = glCreateShader(type);
	glShaderSource(shader, 2, (const char *const *)src_arr, length_arr);
	glCompileShader(shader);

	s32 ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char infoLog[LOG_SIZE];
		glGetShaderInfoLog(shader, LOG_SIZE, NULL, infoLog);
		ErrorStr(infoLog);
		Exit(1);
	}

	return shader;
}

static u32 createProgram(int vertex_src, int fragment_src)
{
	u32 vertex = compileShader(vertex_src, GL_VERTEX_SHADER);
	u32 fragment = compileShader(fragment_src, GL_FRAGMENT_SHADER);

	u32 shader = glCreateProgram();

	glAttachShader(shader, vertex);
	glAttachShader(shader, fragment);

	glLinkProgram(shader);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	s32 ok;
	glGetProgramiv(shader, GL_LINK_STATUS, &ok);
	if (!ok) {
		char infoLog[LOG_SIZE];
		glGetProgramInfoLog(shader, LOG_SIZE, NULL, infoLog);
		ErrorStr(infoLog);
		Exit(1);
	}

	return shader;
}

static void RendererFlush(Renderer *renderer)
{
	s32 length = renderer->sprite_buffer_length;
	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		length * sizeof(QuadVertex) * 4,
		(void *)renderer->sprite_buffer
	);

	glUniformMatrix4fv(renderer->u_transform, 1, GL_FALSE, renderer->transform);
	glDrawElements(GL_TRIANGLES, length * 6, GL_UNSIGNED_INT, 0);

	renderer->sprite_buffer_length = 0;
}

void DrawQuad(Renderer *renderer, vec2 pos, int id)
{
	if (renderer->sprite_buffer_length >= SPRITE_BUFFER_CAPACITY)
		RendererFlush(renderer);

	s32 length = 4 * renderer->sprite_buffer_length++;

	// TODO: Maybe use a LUT?
	float u = (float)(id & ((1 << SPRITESHEET_COLUMNS_EXP) - 1));
	float v = (float)(id >> SPRITESHEET_COLUMNS_EXP);

	float w = 1.0f / (float)(1 << SPRITESHEET_COLUMNS_EXP);
	float h = 1.0f / (float)SPRITESHEET_ROWS;

	float x1 = pos.x;
	float y1 = pos.y;
	float x2 = x1 + (float)SPRITE_DIMENSIONS;
	float y2 = y1 + (float)SPRITE_DIMENSIONS;

	float u1 = u * w;
	float v1 = v * h;
	float u2 = (u + 1.0f) * w;
	float v2 = (v + 1.0f) * h;

	renderer->sprite_buffer[length + 0] = (QuadVertex){
		.pos = {x1, y1},
		.uv  = {u1, v1}
	};

	renderer->sprite_buffer[length + 1] = (QuadVertex){
		.pos = {x2, y1},
		.uv  = {u2, v1}
	};

	renderer->sprite_buffer[length + 2] = (QuadVertex){
		.pos = {x2, y2},
		.uv  = {u2, v2}
	};

	renderer->sprite_buffer[length + 3] = (QuadVertex){
		.pos = {x1, y2},
		.uv  = {u1, v2}
	};
}

void RendererResize(Renderer *renderer, int width, int height)
{
	renderer->width = width;
	renderer->height = height;

	/*
	glBindVertexArray(renderer->screen_vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->screen_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), (void *)v);
	glBindVertexArray(renderer->sprite_vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->sprite_vbo);
	*/
}

void RendererBegin(Renderer *renderer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, renderer->framebuffer);
	glBindVertexArray(renderer->sprite_vao);
	glBindTexture(GL_TEXTURE_2D, renderer->spritesheet);

	glViewport(0, 0, 128, 128);

	glClear(GL_COLOR_BUFFER_BIT);
}

void RendererEnd(Renderer *renderer)
{
	RendererFlush(renderer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(renderer->screen_vao);
	glBindTexture(GL_TEXTURE_2D, renderer->framebuffer_texture);

	glViewport(0, 0, renderer->width, renderer->height);
	glUniformMatrix4fv(renderer->u_transform, 1, GL_FALSE, renderer->identity);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void CameraMove(Renderer *renderer, vec2 pos)
{
	renderer->transform[12] = -pos.x*renderer->transform[0];
	renderer->transform[13] = -pos.y*renderer->transform[5];
}

#ifndef GL_ES
static void debugCallback(GLenum source, GLenum type, u32 id, GLenum severity, GLsizei len, const char *message, const void *userParam)
{
	Log("GL: %s", message);
}

void RendererEnableDebugLogs(void)
{
	Log("GL Debug messages enabled");
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(debugCallback, NULL);
	//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
}
#endif

static u32 LoadTexture(int asset)
{
	u32 texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	str file = AssetRead(asset);

	int height, width;
	unsigned char *data = ImageDecode(file.data, (int)file.length, &width, &height, 4);
	if (data == NULL) {
		Error("File");
		Exit(1);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	ImageFree(data);

	return texture;
}

void RendererInit(Renderer *renderer, Arena temp)
{
	int width = 128, height = 128;

	Log("OpenGL Version: %s", glGetString(GL_VERSION));

	memset(renderer->transform, 0, sizeof(mat4));
	renderer->transform[0]  =  2.0f / (float)width;
	renderer->transform[5]  = -2.0f / (float)height;
	renderer->transform[10] =  0.0f;
	renderer->transform[15] =  1.0f;

	memset(renderer->identity, 0, sizeof(mat4));
	renderer->identity[0]  = 1.0f;
	renderer->identity[5]  = 1.0f;
	renderer->identity[10] = 1.0f;
	renderer->identity[15] = 1.0f;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Compiling and checking shaders
	u32 program = createProgram(ASSET_VERTEX_SHADER, ASSET_FRAGMENT_SHADER);

	s32 transform = glGetUniformLocation(program, "transform");
	if (transform == -1) {
		Error("Shader: no uniform named \"transform\"");
		Exit(1);
	}

	// Setting up vertex buffers
	u32 buffers[3], sprite_vbo, screen_vbo, ebo;
	u32 vaos[2], sprite_vao, screen_vao;

	Log("%lu", countof(vaos));
	glGenVertexArrays(countof(vaos), vaos);
	glGenBuffers(countof(buffers), buffers);

	sprite_vao = vaos[0];
	screen_vao = vaos[1];

	sprite_vbo = buffers[0];
	screen_vbo = buffers[1];
	ebo = buffers[2];

	size indices_length = SPRITE_BUFFER_CAPACITY * 6;
	u32 *indices = AllocArray(&temp, u32, indices_length);

	for (int i = 0, j = 0; i < indices_length; i += 6, j += 4) {
		indices[i + 0] = j + 0;
		indices[i + 1] = j + 1;
		indices[i + 2] = j + 3;
		indices[i + 3] = j + 1;
		indices[i + 4] = j + 2;
		indices[i + 5] = j + 3;
	}

	// You can't bind an EBO unless a VAO is binded
	QuadVertex v[4] = {
		{
			.pos = {-1.0f, 1.0f},
			.uv = {0.0f, 1.0f}

		},
		{
			.pos = {1.0f, 1.0f},
			.uv = {1.0f, 1.0f}

		},
		{
			.pos = {1.0f, -1.0f},
			.uv = {1.0f, 0.0f}

		},
		{
			.pos = {-1.0f, -1.0f},
			.uv = {0.0f, 0.0f}

		}
	};
	glBindVertexArray(screen_vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indices_length, indices, GL_STATIC_DRAW); 

	glBindBuffer(GL_ARRAY_BUFFER, screen_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertex) * 4, v, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void *)offsetof(QuadVertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void *)offsetof(QuadVertex, uv));
	glEnableVertexAttribArray(1);

	glBindVertexArray(sprite_vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indices_length, indices, GL_STATIC_DRAW); 

	glBindBuffer(GL_ARRAY_BUFFER, sprite_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->sprite_buffer), NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void *)offsetof(QuadVertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void *)offsetof(QuadVertex, uv));
	glEnableVertexAttribArray(1);

	// Loading textures
	u32 spritesheet = LoadTexture(ASSET_SPRITESHEET);

	// Creating framebuffer
	u32 framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	u32 framebuffer_texture;
	glGenTextures(1, &framebuffer_texture);
	glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_texture, 0);

	u32 rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Exit(420);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glUseProgram(program);

	renderer->spritesheet = spritesheet;
	renderer->sprite_vao = sprite_vao;
	renderer->screen_vao = screen_vao;
	renderer->sprite_vbo = sprite_vbo;
	renderer->screen_vbo = screen_vbo;
	renderer->u_transform = transform;
	renderer->framebuffer = framebuffer;
	renderer->framebuffer_texture = framebuffer_texture;
	renderer->sprite_buffer_length = 0;
}
