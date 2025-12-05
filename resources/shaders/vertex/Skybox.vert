#version 400 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    TexCoords = aPos;
    mat4 view = mat4(mat3(u_view)); // Remove translation from view matrix - keep only rotation (disabling this makes the skybox black)
    vec4 pos = u_projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;   // CORRECT for skybox
}