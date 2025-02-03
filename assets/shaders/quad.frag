precision mediump float;

in vec2 v_uv;
in vec2 v_light_distance;

out vec4 f_color;

uniform sampler2D spritesheet;

void main(void)
{
	vec3 light_color = vec3(1.0);
	float max_light_distance = 3.0;
	float light_intensity = 0.8;

	float d = min(length(v_light_distance), max_light_distance) / max_light_distance;
	d = pow(d, 1.0);
	vec4 color = texture(spritesheet, v_uv);
	color.rgb *= (1.0 - d) * light_intensity * light_color;
	f_color = color;
} 
