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

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (oogaboogaInit(__FILE__))
        goto out;

    {
        float vertices[] = {
            -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, // bottom left red
            0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // bottom right blue
            0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f    // top blue
        };

        VertexBuffer vb(vertices, sizeof(vertices));

        VertexBufferLayout layout;
        layout.Push<float>(2); // aPos
        layout.Push<float>(3); // aColor
        layout.Push<float>(1); // moveVertex

        VertexArray va;
        va.AddBuffer(vb, layout);

        unsigned int indices[] = {
            0,
            1,
            2,
        };
        std::cout << "sizeof(indices): " << sizeof(indices) << std::endl;
        IndexBuffer ib(indices, 3, BufferUsage::DYNAMIC_DRAW);

        Shader shader;
        shader.addShader(SHADER_DIR / "color_from_attr.vert", ShaderType::VERTEX);
        shader.addShader(SHADER_DIR / "color_from_attr.frag", ShaderType::FRAGMENT);
        shader.CreateProgram();

        shader.Bind();

        // Unbind everything
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        glfwSetKeyCallback(window, keyCallback);

        Renderer renderer;

        while (!glfwWindowShouldClose(window))
        {
            renderer.Clear();

            shader.Bind(); // we dont need to bind shader for rendering purposes, but for updating the uniforms.

            renderer.Draw(va, ib, shader);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

out:
    oogaboogaExit();

    return 0;
}

void keyCallback(GLFWwindow *wdw, int key, int /*scancode*/, int action, int mods)
{
    // std::cout << "key: " << key << ", action: " << action << ", mods:" << mods << std::endl;
    if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
    {
        glfwSetWindowShouldClose(wdw, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_K && action == GLFW_PRESS)
    {
        num_triangles++;
        std::cout << "num_triangles: " << num_triangles << std::endl;
    }
    else if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        num_triangles--;
        if (num_triangles < 1)
            num_triangles = 1;
        std::cout << "num_triangles: " << num_triangles << std::endl;
    }
}
