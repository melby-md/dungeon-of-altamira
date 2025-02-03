layout (location = 0) in vec3 a_pos;

layout (std140) uniform UBO {
	vec2 u_translate;
	vec2 u_scale;
	vec2 u_light_pos;
};

void main(void)
{
	gl_Position = vec4((a_pos.xy - a_pos.z*u_light_pos - u_translate) * u_scale, 0.0, 1.0 - a_pos.z);
}
