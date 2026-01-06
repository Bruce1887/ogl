#version 410 core
out vec4 FragColor;

uniform vec4 u_flashColor;

void main()
{
    FragColor = u_flashColor;
}
