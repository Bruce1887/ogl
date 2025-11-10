#include <iostream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Common.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include <cmath>

int num_triangles = 1;

void keyCallback(GLFWwindow *wdw, int key, int /*scancode*/, int action, int mods);

std::vector<float> vertices;
VertexBufferLayout layout;

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (oogaboogaInit(__FILE__))
        goto out;

    {
        vertices = {
            -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f};

        VertexBuffer vb(vertices);

        layout.push<float>(2); // aPos
        layout.push<float>(3); // aColor
        layout.push<float>(1); // moveVertex

        VertexArray va;
        va.addBuffer(vb, layout);

        unsigned int indices[] = {
            0,
            1,
            2,
        };

        IndexBuffer ib(indices, 3, BufferUsage::DYNAMIC_DRAW);

        Shader shader;
        shader.addShader(SHADER_DIR / "color_from_attr.vert", ShaderType::VERTEX);
        shader.addShader(SHADER_DIR / "color_from_attr.frag", ShaderType::FRAGMENT);
        shader.createProgram();

        shader.bind();

        // Unbind everything
        va.unbind();
        vb.unbind();
        ib.unbind();
        shader.unbind();

        glfwSetKeyCallback(window, keyCallback);

        Renderer renderer;

        while (!glfwWindowShouldClose(window))
        {
            renderer.clear();

            shader.bind(); // we dont need to bind shader for rendering purposes, but for updating the uniforms.

            renderer.draw(va, ib, shader);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

out:
    oogaboogaExit();

    return 0;
}

void circleVertices(float centerX, float centerY, float radius, int numSegments, std::vector<float> &outVertices)
{
    outVertices.clear();
    for (int i = 0; i < numSegments; ++i)
    {
        float theta = 2.0f * 3.1415926f * float(i) / float(numSegments); // current angle

        float x = radius * cosf(theta); // calculate the x component
        float y = radius * sinf(theta); // calculate the y component

        // Position
        outVertices.push_back(x + centerX);
        outVertices.push_back(y + centerY);

        // Other vertex attributes
        float blue = i % 2 == 0 ? 1.0f : 0.0f;
        float red = 1.0f - blue;
        outVertices.push_back(red);  // r
        outVertices.push_back(0.0f); // g
        outVertices.push_back(blue); // b

        outVertices.push_back(0.0f); // moveVertex
    }
}

void keyCallback(GLFWwindow *wdw, int key, int /*scancode*/, int action, int mods)
{
    // std::cout << "key: " << key << ", action: " << action << ", mods:" << mods << std::endl;
    if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
    {
        glfwSetWindowShouldClose(wdw, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        num_triangles++;
        std::cout << "num_triangles: " << num_triangles << std::endl;
    }
    else if (key == GLFW_KEY_K && action == GLFW_PRESS)
    {
        num_triangles--;
        if (num_triangles < 1)
            num_triangles = 1;
        std::cout << "num_triangles: " << num_triangles << std::endl;
    }

    circleVertices(0.0f, 0.0f, 0.5f, num_triangles, vertices);
    VertexBuffer vb(vertices);

    // va.AddBuffer(vb, layout);
}
