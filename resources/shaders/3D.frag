#version 400 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D u_texture1;
uniform sampler2D u_texture2;

void main()
{
	vec4 tex1 = texture(u_texture1, TexCoord);
	vec4 tex2 = texture(u_texture2, TexCoord);

	// Standard alpha blending
	FragColor = tex1 * (1.0 - tex2.a) + tex2 * tex2.a;
}