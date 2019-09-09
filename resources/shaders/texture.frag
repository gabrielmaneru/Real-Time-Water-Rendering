#version 440
in vec2 uv;

uniform sampler2D uniform_texture;

out vec4 out_color;

void main()
{
	out_color = vec4(texture2D(uniform_texture, uv).rgb, 1.0);
}