// 3D vertex shader WITH texture coordinates
#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
	texCoord = vec2(aTexCoord.x, aTexCoord.y);
}
