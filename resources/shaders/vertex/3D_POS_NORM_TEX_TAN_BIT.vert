// 3D Lighting Vertex Shader. Spits out fragment position and normal in addition to texture coordinates. 
#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 fragPos;
out vec3 normal;
out vec2 texCoord;
out vec3 tangent;
out vec3 bitangent;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    fragPos = vec3(u_model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(u_model))) * aNormal;
    texCoord = aTexCoord;
    tangent = mat3(u_model) * aTangent;
    bitangent = mat3(u_model) * aBitangent;
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}