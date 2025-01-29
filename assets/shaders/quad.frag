precision mediump float;

in vec2 v_uv;
in vec2 v_pos;

out vec4 f_color;

uniform sampler2D u_spritesheet;

layout (std140) uniform UBO {
	mat4 u_transform;
	vec2 u_light_pos;
};

void main(void)
{
	// Pixel art filtering
	vec2 texture_size = vec2(textureSize(u_spritesheet, 0));
	vec2 pixel = v_uv * texture_size;
	vec2 seam = floor(pixel + 0.5);
	vec2 dudv = fwidth(pixel);
	pixel = seam + clamp((pixel - seam) / dudv, -0.5, 0.5);
	vec4 color = texture(u_spritesheet, pixel / texture_size);

	// Lighting
	vec3 light_color = vec3(1.0);
	float light_distance = 3.0;
	float light_intensity = 0.8;

	float d = min(length(v_pos - u_light_pos), light_distance ) / light_distance;
	d = pow(d, 1.0);
	color.rgb *= (1.0 - d) * light_intensity * light_color;

	f_color = color;
} 
