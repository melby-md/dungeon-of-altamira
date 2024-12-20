precision mediump float;

in vec2 v_uv;

out vec4 f_color;

uniform sampler2D spritesheet;

void main(void)
{
	f_color = texture(spritesheet, v_uv);
} 
