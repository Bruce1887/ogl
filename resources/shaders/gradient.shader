#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;

out vec4 vertexColor; // will be smoothly interpolated across fragments

void main()
{
    gl_Position = vec4(aPos, 1.0);

    // Map position from [-1, 1] to [0, 1] range to make gradient visually nice
    float red   = (aPos.x + 1.0) * 0.5;  // left (-1) = 0.0, right (+1) = 1.0
    float green  = 0.2;                   // fixed green blue value for contrast
    float blue = (aPos.y + 1.0) * 0.5;  // bottom (-1) = 0.0, top (+1) = 1.0

    vertexColor = vec4(red, green, blue, 1.0);
}

#shader fragment
#version 330 core
out vec4 FragColor;
  
in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  

void main()
{
    FragColor = vertexColor;
} 