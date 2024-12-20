layout (location = 0) in vec3 a_pos;

layout (std140) uniform UBO {
	mat4 u_transform;
	vec2 u_light_pos;
};

void main(void)
{
	gl_Position = u_transform * vec4(a_pos.xy - a_pos.z*u_light_pos, 0.0, 1.0 - a_pos.z);
}
