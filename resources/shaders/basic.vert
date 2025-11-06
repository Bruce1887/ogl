#version 400 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 texCoord;

out vec3 v_color;
out vec2 v_texCoord;

void main()
{
   gl_Position = position;
   v_color = aColor;
   v_texCoord = texCoord;
}