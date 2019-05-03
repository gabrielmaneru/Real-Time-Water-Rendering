#version 440
in vec2 var_uv;
out vec4 out_color;
uniform sampler2D uniform_texture;

void main()
{
	vec3 texel = texture2D(uniform_texture, var_uv).rgb;
	out_color = vec4(texel, 1.0f);
}