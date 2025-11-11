#include "Common.h"
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

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (oogaboogaInit(__FILE__))
        goto out;

    {
        float vertices[] = {
            -0.5f,
            -0.5f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f, // bottom left

            0.5f,
            -0.5f,
            0.0f,
            1.0f,
            0.0f,
            1.0f,
            0.0f, // bottom right

            0.5f,
            0.5f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f, // top right

            -0.5f,
            0.5f,
            1.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f, // top left
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0};

        VertexBuffer vb(vertices, sizeof(vertices));

        VertexBufferLayout layout;
        layout.push<float>(2);
        layout.push<float>(3);
        layout.push<float>(2);

        VertexArray va;
        va.addBuffer(vb, layout);
        vb.unbind(); // can unbind VB after adding to VAO
        
        IndexBuffer ib(indices, 6);

        Shader horizontal_shader;
        horizontal_shader.addShader(SHADER_DIR / "horizontal.vert", ShaderType::VERTEX);
        horizontal_shader.addShader(SHADER_DIR / "horizontal.frag", ShaderType::FRAGMENT);
        horizontal_shader.createProgram();
        horizontal_shader.bind();

        Texture texture1((TEXTURE_DIR / "lennart.jpg").string(), 0);
        texture1.bind();
        horizontal_shader.setUniform("u_texture1", 0);

        Texture texture2((TEXTURE_DIR / "alf.png").string(), 1);
        texture2.bind();
        horizontal_shader.setUniform("u_texture2", 1);
        horizontal_shader.unbind();

        Shader spinning_shader;
        spinning_shader.addShader(SHADER_DIR / "spinning.vert", ShaderType::VERTEX);
        spinning_shader.addShader(SHADER_DIR / "spinning.frag", ShaderType::FRAGMENT);
        spinning_shader.createProgram();
        spinning_shader.bind();
        spinning_shader.setUniform("u_tex_scale", 3.0f);

        texture1.bind();
        spinning_shader.setUniform("u_texture1", 0);
        texture2.bind();
        spinning_shader.setUniform("u_texture2", 1);
        spinning_shader.unbind();
        

        while (!glfwWindowShouldClose(window))
        {
            GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            horizontal_shader.bind();
            horizontal_shader.setUniform("u_time", (float)glfwGetTime());

            GLCALL(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, nullptr));

            spinning_shader.bind();
            spinning_shader.setUniform("u_time", (float)glfwGetTime());

            GLCALL(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, nullptr));

            glfwSwapBuffers(window);

            glfwPollEvents();
        }
    }

out:
    oogaboogaExit();

    return 0;
}
