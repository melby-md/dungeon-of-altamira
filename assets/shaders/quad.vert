layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

layout (std140) uniform UBO {
	mat4 u_transform;
	vec2 u_light_pos;
};

out vec2 v_uv;

void main(void)
{
	v_uv = a_uv;
	gl_Position = u_transform * vec4(a_pos, 0.0, 1.0);
}
