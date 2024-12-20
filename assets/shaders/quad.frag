precision mediump float;

in vec2 v_uv;
in vec2 v_pos;

out vec4 f_color;

layout (std140) uniform UBO {
	mat4 u_transform;
	vec2 u_light_pos;
};

uniform sampler2D spritesheet;

void main(void)
{
	vec3 light_color = vec3(1.0);
	float light_distance = 3.0;
	float light_intensity = 0.8;

	float d = min(length(v_pos-u_light_pos), light_distance ) / light_distance;
	d = pow(d,1.0);
	vec4 color = texture(spritesheet, v_uv);
	color.rgb *= (1.0-d) * light_intensity * light_color;
	f_color = color;
} 
