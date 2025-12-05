#version 400 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_cubemap;

void main()
{    
    FragColor = texture(u_cubemap, TexCoords);
}