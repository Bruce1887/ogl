#version 400 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

out vec3 v_color;
out vec2 v_texCoord;

uniform float u_time;

void main()
{
   mat4 rotation = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        sin(u_time)/2.0, 0.0, 0.0, 1.0
    );
   gl_Position = rotation * vec4(aPos, 0.0f, 1.0f);
   v_color = aColor;
   v_texCoord = aTexCoord;
}