#include <stddef.h> // offsetof

#ifdef GLAD
#  include <glad/gl.h>
#else
#  ifdef GL_ES
#    include <GLES3/gl32.h>
#  else
#    define GL_GLEXT_PROTOTYPES
#    include <GL/gl.h>
#  endif
#endif

#include "arena.h"
#include "assets.h"
#include "common.h"
#include "config.h"
#include "image.h"
#include "platform.h"
#include "renderer.h"
#include "renderer_gl.h"

#ifdef GL_ES
static char shader_header[] = "#version 300 es\n";
#else
static char shader_header[] = "#version 330 core\n";
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
		Panic(infoLog);
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
		Panic(infoLog);
	}

	return shader;
}

static void pushQuad(Renderer *renderer, vec2 pos, int id) {
	QuadVertex *quad = renderer->sprite_buffer[renderer->sprite_buffer_length++];

	// TODO: Maybe use a LUT?
	float u = (float)(id & ((1 << SPRITESHEET_COLUMNS_EXP) - 1));
	float v = (float)(id >> SPRITESHEET_COLUMNS_EXP);

	float w = 1.f / (float)(1 << SPRITESHEET_COLUMNS_EXP);
	float h = 1.f / (float)SPRITESHEET_ROWS;

	float x1 = pos.x;
	float y1 = pos.y;
	float x2 = x1 + 1.f;
	float y2 = y1 + 1.f;

	float u1 = u * w;
	float v1 = v * h;
	float u2 = (u + 1.f) * w;
	float v2 = (v + 1.f) * h;

	quad[0] = (QuadVertex){
		.pos = {x1, y1},
		.uv  = {u1, v1}
	};

	quad[1] = (QuadVertex){
		.pos = {x2, y1},
		.uv  = {u2, v1}
	};

	quad[2] = (QuadVertex){
		.pos = {x2, y2},
		.uv  = {u2, v2}
	};

	quad[3] = (QuadVertex){
		.pos = {x1, y2},
		.uv  = {u1, v2}
	};
}

void BeginStaticTiles(Renderer *renderer)
{
	Assert(renderer->sprite_buffer_length == 0);
	renderer->shadow_buffer_length = 0;
}

void PushTile(Renderer *renderer, vec2 pos, int id)
{
	if (renderer->sprite_buffer_length >= SPRITE_BUFFER_CAPACITY)
		Panic("Too much tiles");

	pushQuad(renderer, pos, id);
}

void PushShadow(Renderer *renderer, vec2 p1, vec2 p2)
{
	if (renderer->shadow_buffer_length >= SPRITE_BUFFER_CAPACITY)
		Panic("Shdows");

	ShadowVertex *shadow = renderer->shadow_buffer[renderer->shadow_buffer_length++];

	shadow[0] = (ShadowVertex){
		.pos = {p1.x, p1.y, 0.f},
	};

	shadow[1] = (ShadowVertex){
		.pos = {p2.x, p2.y, 0.f},
	};

	shadow[2] = (ShadowVertex){
		.pos = {p2.x, p2.y, 1.f},
	};

	shadow[3] = (ShadowVertex){
		.pos = {p1.x, p1.y, 1.f},
	};
}

void EndStaticTiles(Renderer *renderer)
{
	s32 tile_length = renderer->sprite_buffer_length;
	renderer->sprite_buffer_length = 0;
	renderer->static_tiles_length = tile_length * 6;

	glBindBuffer(GL_ARRAY_BUFFER, renderer->static_tiles_vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(Quad) * tile_length,
		renderer->sprite_buffer,
		GL_STATIC_DRAW
	);

	s32 shadow_length = renderer->shadow_buffer_length;
	glBindBuffer(GL_ARRAY_BUFFER, renderer->shadow_vbo);
	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		shadow_length * sizeof(Shadow),
		(void *)renderer->shadow_buffer
	);
	renderer->shadow_buffer_length = shadow_length * 6;

	glBindBuffer(GL_ARRAY_BUFFER, renderer->sprite_vbo);
}

static void flushQuads(Renderer *renderer)
{
	s32 length = renderer->sprite_buffer_length;
	glBufferSubData(
		GL_ARRAY_BUFFER,
		0,
		length * sizeof(Quad),
		(void *)renderer->sprite_buffer
	);

	glDrawElements(GL_TRIANGLES, length * 6, GL_UNSIGNED_INT, 0);

	renderer->sprite_buffer_length = 0;
}

void BeginCamera(Renderer *renderer, vec2 camera)
{
	camera = vec2_addf(camera, .5f);

	glUseProgram(renderer->quad_shader);

	renderer->transform[12] = -camera.x*renderer->transform[0];
	renderer->transform[13] = -camera.y*renderer->transform[5];
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), renderer->transform);
	float pos[2] = {camera.x, camera.y};
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(pos), pos);

	glBindVertexArray(renderer->static_tiles_vao);
	glDrawElements(GL_TRIANGLES, renderer->static_tiles_length, GL_UNSIGNED_INT, 0);

	glBindVertexArray(renderer->sprite_vao);
}

void DrawSprite(Renderer *renderer, vec2 pos, int id)
{
	if (renderer->sprite_buffer_length >= SPRITE_BUFFER_CAPACITY)
		flushQuads(renderer);
	
	pushQuad(renderer, pos, id);
}

void EndCamera(Renderer *renderer)
{
	flushQuads(renderer);

	glUseProgram(renderer->shadow_shader);

	glBindVertexArray(renderer->shadow_vao);
	glDrawElements(GL_TRIANGLES, renderer->shadow_buffer_length, GL_UNSIGNED_INT, 0);
}

void BeginRender(Renderer *renderer)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer->framebuffer);
	glClear(GL_COLOR_BUFFER_BIT);
}

void EndRender(Renderer *renderer)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlitFramebuffer(
		0, 0, CANVAS_WIDTH, CANVAS_HEIGHT,
		renderer->left, renderer->top, renderer->right, renderer->bottom,
		GL_COLOR_BUFFER_BIT, GL_NEAREST
	);
}

void RendererResize(Renderer *renderer, int width, int height)
{
	// TODO: solve letterbox issues
	int scale_factor = (width / CANVAS_WIDTH);
	int letterbox_x = (width % CANVAS_WIDTH)/2;
	int letterbox_y = (height % CANVAS_HEIGHT)/2;

	renderer->left = letterbox_x;
	renderer->top = letterbox_y;
	renderer->right = letterbox_x + CANVAS_WIDTH * scale_factor;
	renderer->bottom = letterbox_y + CANVAS_HEIGHT * scale_factor;

	renderer->width = width;
	renderer->height = height;
}

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
		Panic("File");
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	ImageFree(data);

	return texture;
}

void debugOutput(
	GLenum source, 
	GLenum type, 
	unsigned int id, 
	GLenum severity, 
	GLsizei length, 
	const char *message, 
	const void *userParam
)
{
	Log("GL: %s", message);
}

void RendererInit(Renderer *renderer, Arena temp, bool gl_debug)
{
	Log("OpenGL Version: %s", glGetString(GL_VERSION));

	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		Log("GL: Debug context enabled");
	} 

	if (gl_debug) {
		Log("GL: Debug output enabled");
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
		glDebugMessageCallback(debugOutput, NULL);
		glDebugMessageControl(
			GL_DONT_CARE, 
			GL_DONT_CARE, 
			GL_DONT_CARE,
			0,
			NULL,
			GL_TRUE
		); 
	}

	memset(renderer->transform, 0, sizeof(mat4));
	renderer->transform[0]  =  2.0f / ((float)CANVAS_WIDTH / (float)SPRITE_DIMENSION);
	renderer->transform[5]  = -2.0f / ((float)CANVAS_HEIGHT / (float)SPRITE_DIMENSION);
	renderer->transform[15] =  1.0f;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Compiling and checking shaders
	u32 quad_shader = createProgram(ASSET_QUAD_VERTEX_SHADER, ASSET_QUAD_FRAGMENT_SHADER);
	u32 shadow_shader = createProgram(ASSET_SHADOW_VERTEX_SHADER, ASSET_SHADOW_FRAGMENT_SHADER);

	s32 ubo_binding = glGetUniformBlockIndex(quad_shader, "UBO");
	if (ubo_binding == -1) {
		Panic("Shader: no uniform block named \"UBO\"");
	}
	glUniformBlockBinding(quad_shader, ubo_binding, 0);

	ubo_binding = glGetUniformBlockIndex(shadow_shader, "UBO");
	if (ubo_binding == -1) {
		Panic("Shader: no uniform block named \"UBO\"");
	}
	glUniformBlockBinding(shadow_shader, ubo_binding, 0);

	// sizeof(mat4) + sizeof(vec2)
	// no need for padding
	static const size ubo_length = 72;
	u32 ubo;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, ubo_length, NULL, GL_DYNAMIC_DRAW);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, ubo_length);

	// Setting up vertex buffers
	u32 buffers[4], sprite_vbo, static_tiles_vbo, shadow_vbo, ebo;
	u32 vaos[3], sprite_vao, static_tiles_vao, shadow_vao;

	glGenVertexArrays(countof(vaos), vaos);
	glGenBuffers(countof(buffers), buffers);

	sprite_vao = vaos[0];
	static_tiles_vao = vaos[1];
	shadow_vao = vaos[2];

	sprite_vbo = buffers[0];
	static_tiles_vbo = buffers[1];
	shadow_vbo = buffers[2];
	ebo = buffers[3];

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

	glBindVertexArray(static_tiles_vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indices_length, indices, GL_STATIC_DRAW); 

	glBindBuffer(GL_ARRAY_BUFFER, static_tiles_vbo);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void *)offsetof(QuadVertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void *)offsetof(QuadVertex, uv));
	glEnableVertexAttribArray(1);

	glBindVertexArray(shadow_vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBindBuffer(GL_ARRAY_BUFFER, shadow_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->shadow_buffer), NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ShadowVertex), (void *)offsetof(ShadowVertex, pos));
	glEnableVertexAttribArray(0);

	glBindVertexArray(sprite_vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

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

	u32 framebuffer_color;
	glGenTextures(1, &framebuffer_color);
	glBindTexture(GL_TEXTURE_2D, framebuffer_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CANVAS_WIDTH, CANVAS_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_color, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Panic("Error while creating framebuffer");

	glViewport(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
	glClearColor(0.f, 0.f, 0.f, 1.f);

	glBindTexture(GL_TEXTURE_2D, spritesheet);

	renderer->quad_shader = quad_shader;
	renderer->shadow_shader = shadow_shader;
	renderer->framebuffer = framebuffer;
	renderer->sprite_vao = sprite_vao;
	renderer->sprite_vbo = sprite_vbo;
	renderer->static_tiles_vbo = static_tiles_vbo;
	renderer->static_tiles_vao = static_tiles_vao;
	renderer->shadow_vbo = shadow_vbo;
	renderer->shadow_vao = shadow_vao;
	renderer->sprite_buffer_length = 0;
	renderer->shadow_buffer_length = 0;
}
