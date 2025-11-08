#version 400 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D u_texture1;
uniform sampler2D u_texture2;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	// FragColor = mix(texture(u_texture1, TexCoord), texture(u_texture2, TexCoord), 0.2);

	FragColor = texture(u_texture1, TexCoord) + texture(u_texture2, TexCoord) * 0.4;
}