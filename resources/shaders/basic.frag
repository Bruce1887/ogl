#version 400 core

layout(location = 0) out vec4 color;
in vec2 v_texCoord;
uniform vec4 u_color;
uniform sampler2D u_texture;
void main()
{
   // color = texture(u_texture, v_texCoord);   
   vec4 texColor = texture(u_texture, v_texCoord);
   color = texColor * u_color;
}