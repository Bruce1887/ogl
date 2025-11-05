#version 460 core
layout (location = 0) in vec2 aPos;   // the position variable has attribute position 0
layout (location = 1) in vec3 aColor; // the color variable has attribute position 1
layout (location = 2) in float moveVertex; // the color variable has attribute position 1

out vec3 ourColor; // output a color to the fragment shader
uniform float h_offset;
uniform float v_offset;
void main()
{
    if (moveVertex > 0.5){
        
    gl_Position = vec4(
        aPos.x+h_offset,
        aPos.y+v_offset,
        0.0, // aPos.z,
        1.0
        );
    }
    ourColor = aColor; // set ourColor to the input color we got from the vertex data
}