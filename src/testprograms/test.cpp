#include <iostream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include <cmath>

#include <Common.h>

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (init())
        goto out;

    {
        float vertices[] = {
            -0.5f,-0.5f,1.0f,0.0f,0.0f, // bottom left red
            0.5f,-0.5f,0.0f,1.0f,0.0f, // bottom right green
            0.0f,0.5f,0.0f,0.0f,1.0f, // top blue
        };

        unsigned int indices[] = {
            0, 1, 2};

        VertexBuffer vb(vertices, sizeof(vertices));

        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(3);

        VertexArray va;
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

        Shader shader;
        shader.addShader(SHADER_DIR "color_from_attr.vert", ShaderType::VERTEX);
        shader.addShader(SHADER_DIR "color_from_attr.frag", ShaderType::FRAGMENT);
        shader.CreateProgram();

        shader.Bind();

        // Unbind everything
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();
        float hoff = 0.0;
        Renderer renderer;


        while (!glfwWindowShouldClose(window))
        {
            renderer.Clear();
            shader.SetUniform1f("h_offset",hoff);
            hoff = (sin(glfwGetTime()) / 2.0f);
            shader.Bind();

            renderer.Draw(va, ib, shader);

            glfwSwapBuffers(window);

            glfwPollEvents();
        }
    }

    glfwTerminate(); // dont call glfwTerminate() if glfw failed to initialize, thats handled internally in GLFW
out:
    exit();

    return 0;
}
