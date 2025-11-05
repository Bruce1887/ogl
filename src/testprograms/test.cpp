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

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (oogaboogaInit(__FILE__))
        goto out;
    {
        float vertices[] = {
            -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, // bottom left red
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom right green
            0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f    // top blue (left)
        };
        unsigned int indices[] = {
            0,
            1,
            2,
        };

        VertexBuffer vb(vertices, sizeof(vertices));

        VertexBufferLayout layout;
        layout.Push<float>(2); // aPos
        layout.Push<float>(3); // aColor
        layout.Push<float>(1); // moveVertex

        VertexArray va;
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 3, BufferUsage::STATIC_DRAW);

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

        Renderer renderer;

        while (!glfwWindowShouldClose(window))
        {
            renderer.Clear();

            shader.Bind(); // we dont need to bind shader for rendering purposes, but for updating the uniforms.
            shader.SetUniform1f("h_offset", (sin(glfwGetTime()) / 2.0f));
            shader.SetUniform1f("v_offset", (cos(glfwGetTime()) / 2.0f));
            ;

            renderer.Draw(va, ib, shader);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

out:
    oogaboogaExit();

    return 0;
}
