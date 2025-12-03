#version 400 core

layout (location = 0) in vec2 aPos;

uniform mat4 u_Projection; // Orthographic matrix (screen space)
uniform mat4 u_Model;      // Translation and Scale matrix (position and size of the HUD element)

void main()
{

    gl_Position = u_Projection * u_Model * vec4(aPos.x, aPos.y, 0.0, 1.0);
}