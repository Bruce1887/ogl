// 2 texture fragment shader with linear interpolation and a mix value
#version 400 core
out vec4 FragColor;

in vec2 TexCoord;

uniform float mixValue;

// texture sampler
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixValue);
}