// wonky ass vertex shader
#version 400 core
layout (location = 0) in vec2 aPos;   // the position variable has attribute position 0
layout (location = 1) in vec3 aColor; // the color variable has attribute position 1
layout (location = 2) in float moveVertex; // the moveVertex variable has attribute position 2

out vec3 varColor; // output a color to the fragment shader

uniform float h_offset;
uniform float v_offset;

void main()
{
    float xPos = aPos.x;
    float yPos = aPos.y;

    if (moveVertex > 0.5) {
        xPos += h_offset;
        yPos += v_offset;
    }

    gl_Position = vec4(
        xPos,
        yPos,
        0.0,
        1.0
    );

    varColor = aColor;
}
