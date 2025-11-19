// 3D Lighting Vertex Shader with vertex color support
#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

out vec3 fragPos;
out vec3 normal;
out vec3 vertColor;
out vec2 texCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    fragPos = vec3(u_model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(u_model))) * aNormal;
    vertColor = aColor;
    texCoord = aTexCoord;
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}
