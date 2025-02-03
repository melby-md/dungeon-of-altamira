layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

layout (std140) uniform UBO {
	vec2 u_translate;
	vec2 u_scale;
	vec2 u_light_pos;
};

out vec2 v_uv;
out vec2 v_light_distance;

void main(void)
{
	v_uv = a_uv;
	v_light_distance = a_pos - u_light_pos;
	gl_Position = vec4((a_pos - u_translate) * u_scale, 0.0, 1.0);
}
