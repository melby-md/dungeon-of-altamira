precision mediump float;

out vec4 f_color;

in vec2 v_uv;

uniform sampler2D spritesheet;

void main(void)
{
    f_color = texture(spritesheet, v_uv);
} 
