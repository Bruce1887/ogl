// Instanced Tree Vertex Shader WITH FOG
#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// Instance data (per-tree transform)
layout (location = 5) in vec4 aInstanceRow0;
layout (location = 6) in vec4 aInstanceRow1;
layout (location = 7) in vec4 aInstanceRow2;
layout (location = 8) in vec4 aInstanceRow3;

out vec3 fragPos;
out vec3 normal;
out vec2 texCoord;
out vec3 tangent;
out vec3 bitangent;
out float fogDistance;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_camPos;

void main()
{
    // Reconstruct instance model matrix from columns
    mat4 instanceModel = mat4(aInstanceRow0, aInstanceRow1, aInstanceRow2, aInstanceRow3);
    
    fragPos = vec3(instanceModel * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(instanceModel))) * aNormal;
    texCoord = aTexCoord;
    tangent = mat3(instanceModel) * aTangent;
    bitangent = mat3(instanceModel) * aBitangent;
    
    // Calculate distance from camera for fog
    fogDistance = length(u_camPos - fragPos);
    
    gl_Position = u_projection * u_view * instanceModel * vec4(aPos, 1.0);
}
