#include <assert.h> // static_assert
#include <stdio.h>

#include "config.h"
#include "platform.h"
#include "renderer.h"
#include "renderer_gl.h"
#include "assets.h"
#include "../third-party/stb_image.h"

#ifdef GL_ES
#  include <GLES3/gl3.h>
#  define SHADER_VERSION "#version 300 es\n"
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/gl.h>
#  define SHADER_VERSION "#version 330 core\n"
#endif

#define SHADER(...) SHADER_VERSION #__VA_ARGS__

#define LOG_SIZE 1024

enum assets {
	ASSET_SPRITE_VERTEX_SHADER,
	ASSET_SPRITE_FRAGMENT_SHADER,

	ASSET_SPRITESHEET
};

static const char vertex_src[] = SHADER(
	layout (location = 0) in vec2 pos;
	layout (location = 1) in vec2 uv;

	uniform mat4 transform;

	out vec2 v_uv;

	void main(void)
	{
		v_uv = uv;
		gl_Position = transform * vec4(pos, 0.0, 1.0);
	}
);

static const char fragment_src[] = SHADER(
	precision mediump float;
	out vec4 FragColor;

	uniform sampler2D spritesheet;
	in vec2 v_uv;

	void main(void)
	{
	    FragColor = texture(spritesheet, v_uv);
	} 
);

static_assert(sizeof(GLfloat) == sizeof(float));

static u32 createShader(str src, GLenum type)
{
	u32 shader = glCreateShader(type);
	glShaderSource(shader, 2, (const char *const *)&src.data, (s32 *)&src.length);
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

static u32 createProgram(u32 vertex, u32 fragment)
{
	u32 program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);

	s32 ok;
	glGetProgramiv(program, GL_LINK_STATUS, &ok);
	if (!ok) {
		char infoLog[LOG_SIZE];
		glGetProgramInfoLog(program, LOG_SIZE, NULL, infoLog);
		ErrorStr(infoLog);
		Exit(1);
	}

	return program;
}

void LoadShaders(Renderer *renderer) {
	u32 vertex = createShader(str(vertex_src), GL_VERTEX_SHADER);
	u32 fragment = createShader(str(fragment_src), GL_FRAGMENT_SHADER);

	u32 program = createProgram(vertex, fragment);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	glUseProgram(program);

	s32 transform = glGetUniformLocation(program, "transform");
	if (transform == -1) {
		Error("Shader: no uniform named \"transform\"");
		Exit(1);
	}

	renderer->u_transform = transform;
	renderer->program = program;
}

void RendererResize(Renderer *renderer, int width, int height)
{
	glViewport(0, 0, width, height);
}

void DrawQuad(Renderer *renderer, const vec2 pos, int id)
{
	size length = renderer->buffer_length * 4 * 4;

	float num = 0.25f;

	memcpy(&renderer->quad_buffer[length + 0], pos, sizeof(vec2));
	memcpy(&renderer->quad_buffer[length + 2], vec2(num * (float)id, 0.0f), sizeof(vec2));

	vec2_add(&renderer->quad_buffer[length + 4], pos, vec2(100.0f, 0.0f));
	memcpy(&renderer->quad_buffer[length + 6], vec2(num * (float)(id + 1), 0.0f), sizeof(vec2));

	vec2_add(&renderer->quad_buffer[length + 8], pos, vec2(100.0f, 100.0f));
	memcpy(&renderer->quad_buffer[length + 10], vec2(num * (float)(id + 1), 1.0f), sizeof(vec2));

	vec2_add(&renderer->quad_buffer[length + 12], pos, vec2(0.0f, 100.0f));
	memcpy(&renderer->quad_buffer[length + 14], vec2(num * (float)id, 1.0f), sizeof(vec2));

	if (++renderer->buffer_length >= BUFFER_CAPACITY)
		RendererFlush(renderer);
}

void RendererFlush(Renderer *renderer)
{
	s32 length = renderer->buffer_length;
	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		length * sizeof(float) * 4 * 4,
		renderer->quad_buffer
	);

	glUniformMatrix4fv(renderer->u_transform, 1, GL_FALSE, renderer->transform);
	glDrawElements(GL_TRIANGLES, length * 6, GL_UNSIGNED_INT, 0);

	renderer->buffer_length = 0;
}

void CameraMove(Renderer *renderer, const vec2 pos) {
	renderer->transform[12] = -pos[0]*renderer->transform[0];
	renderer->transform[13] = -pos[1]*renderer->transform[5];
}

void CameraResize(Renderer *renderer, float width, float height) {
	renderer->transform[0] = 2.0f/width;
	renderer->transform[5] = -2.0f/height;
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

void RendererSetClearColor(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

void RendererClear(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void RendererInit(Renderer *renderer)
{
	Log("OpenGL Version: %s", glGetString(GL_VERSION));

	memset(renderer->transform, 0, sizeof(mat4));
	renderer->transform[15] = 1.0f;

	LoadShaders(renderer);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	u32 texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	str file = AssetRead(ASSET_SPRITESHEET);

	int height, width, nrChannels;
	unsigned char *data = stbi_load_from_memory(file.data, (int)file.length, &width, &height, &nrChannels, 4);
	if (data == NULL) {
		Error("File");
		Exit(1);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	u32 buffers[2], vbo, ebo, vao;
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, buffers);
	vbo = buffers[0];
	ebo = buffers[1];

	u32 indices[BUFFER_CAPACITY * 6];

	for (int i = 0, j = 0; i < BUFFER_CAPACITY * 6; i += 6, j += 4) {
		indices[i + 0] = j + 0;
		indices[i + 1] = j + 1;
		indices[i + 2] = j + 3;
		indices[i + 3] = j + 1;
		indices[i + 4] = j + 2;
		indices[i + 5] = j + 3;
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->quad_buffer), NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void *)(sizeof(float) * 2));
	glEnableVertexAttribArray(1);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	renderer->vbo = vbo;
	renderer->buffer_length = 0;
}
