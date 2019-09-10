#version 440
in vec2 uv;

uniform sampler2D texture_diffuse1;

out vec4 out_color;

void main()
{
	out_color = vec4(texture(texture_diffuse1, uv).rgb, 1.0);
	out_color = vec4(0.0, 0.6, 0.0, 1.0);
}