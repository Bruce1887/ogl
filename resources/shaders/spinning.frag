#version 400 core

layout(location = 0) out vec4 color;
in vec2 v_texCoord;
in vec3 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture1;
uniform sampler2D u_texture2;

void main()
{
   color = mix(texture(u_texture1, v_texCoord), texture(u_texture2, v_texCoord), 0.4);
}