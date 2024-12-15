layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

uniform mat4 transform;

out vec2 v_uv;

void main(void)
{
	v_uv = a_uv;
	gl_Position = transform * vec4(a_pos, 0.0, 1.0);
}
