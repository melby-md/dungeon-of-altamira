#include <stddef.h> // offsetof

#include "image.h"
#include "config.h"
#include "platform.h"
#include "renderer.h"
#include "renderer_gl.h"
#include "assets.h"

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

// TODO: find a better way to do this
// number of columns = 1 << TEXTURE_EXP
#define TEXTURE_EXP 2
// number of rows
#define TEXTURE_H 1

static const char vertex_src[] = SHADER(
	layout (location = 0) in vec2 a_pos;
	layout (location = 1) in vec2 a_uv;

	uniform mat4 transform;

	out vec2 v_uv;

	void main(void)
	{
		v_uv = a_uv;
		gl_Position = transform * vec4(a_pos, 0.0, 1.0);
	}
);

static const char fragment_src[] = SHADER(
	precision mediump float;

	out vec4 f_color;

	in vec2 v_uv;

	uniform sampler2D spritesheet;

	void main(void)
	{
	    f_color = texture(spritesheet, v_uv);
	} 
);

static u32 createShader(str src, GLenum type)
{
	u32 shader = glCreateShader(type);
	glShaderSource(shader, 1, (const char *const *)&src.data, (s32 *)&src.length);
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

// TODO: Load shader from file and enable hot reloading
void ReloadShaders(Renderer *renderer) {
	glUseProgram(0);
	glDeleteProgram(renderer->program);

	LoadShaders(renderer);
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

void DrawQuad(Renderer *renderer, vec2 pos, float dim, int id)
{
	if (renderer->quad_buffer_length >= QUAD_BUFFER_CAPACITY)
		RendererFlush(renderer);

	s32 length = 4 * renderer->quad_buffer_length++;

	// TODO: Maybe use a LUT?
	float u = (float)(id & ((1 << TEXTURE_EXP) - 1));
	float v = (float)(id >> TEXTURE_EXP);

	float w = 1.0f / (float)(1 << TEXTURE_EXP);
	float h = 1.0f / (float)TEXTURE_H;

	float x1 = pos.x;
	float y1 = pos.y;
	float x2 = x1 + dim;
	float y2 = y1 + dim;

	float u1 = u * w;
	float v1 = v * h;
	float u2 = (u + 1.0f) * w;
	float v2 = (v + 1.0f) * h;

	renderer->quad_buffer[length + 0] = (QuadVertex){
		.pos = {x1, y1},
		.uv  = {u1, v1}
	};

	renderer->quad_buffer[length + 1] = (QuadVertex){
		.pos = {x2, y1},
		.uv  = {u2, v1}
	};

	renderer->quad_buffer[length + 2] = (QuadVertex){
		.pos = {x2, y2},
		.uv  = {u2, v2}
	};

	renderer->quad_buffer[length + 3] = (QuadVertex){
		.pos = {x1, y2},
		.uv  = {u1, v2}
	};
}

void RendererFlush(Renderer *renderer)
{
	s32 length = renderer->quad_buffer_length;
	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		length * sizeof(QuadVertex) * 4,
		(void *)renderer->quad_buffer
	);

	glUniformMatrix4fv(renderer->u_transform, 1, GL_FALSE, renderer->transform);
	glDrawElements(GL_TRIANGLES, length * 6, GL_UNSIGNED_INT, 0);

	renderer->quad_buffer_length = 0;
}

void CameraMove(Renderer *renderer, vec2 pos) {
	renderer->transform[12] = -pos.x*renderer->transform[0];
	renderer->transform[13] = -pos.y*renderer->transform[5];
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

void SpritesheetLoad(Renderer *renderer, int asset)
{
	u32 texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
}

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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	LoadShaders(renderer);

	u32 buffers[2], vbo, ebo, vao;
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, buffers);
	vbo = buffers[0];
	ebo = buffers[1];

	u32 indices[QUAD_BUFFER_CAPACITY * 6];

	for (int i = 0, j = 0; i < QUAD_BUFFER_CAPACITY * 6; i += 6, j += 4) {
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

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void *)(offsetof(QuadVertex, pos)));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void *)(offsetof(QuadVertex, uv)));
	glEnableVertexAttribArray(1);

	renderer->quad_buffer_length = 0;
}
