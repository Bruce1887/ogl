// sets color from vertex shader output
#version 400 core
out vec4 FragColor;  
in vec3 varColor;

void main()
{
    FragColor = vec4(varColor, 1.0);
}