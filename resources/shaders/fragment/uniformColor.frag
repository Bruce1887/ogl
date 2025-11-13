// sets color from a uniform variable
#version 400 core
out vec4 FragColor;
uniform vec4 u_color;

void main()
{
	// Standard just a white color
	FragColor = u_color;
}