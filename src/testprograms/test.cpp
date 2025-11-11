#include <iostream>

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

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (oogaboogaInit(__FILE__))
        goto out;
        
    // glfw: initialize and configure
    // ------------------------------
    {
        float vertices[] = {
            -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, // bottom left red
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom right green
            0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f    // top blue (left)
        };
        unsigned int indices[] = {
            0,
            1,
            2,
        };

        VertexBuffer vb(vertices, sizeof(vertices));

        VertexBufferLayout layout;
        layout.push<float>(2); // aPos
        layout.push<float>(3); // aColor
        layout.push<float>(1); // moveVertex

        VertexArray va;
        va.addBuffer(vb, layout);

        IndexBuffer ib(indices, 3);

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

        while (!glfwWindowShouldClose(window))
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.bind(); // we dont need to bind shader for rendering purposes, but for updating the uniforms.
            shader.setUniform("h_offset", (float)(sin(glfwGetTime()) / 2.0f));
            shader.setUniform("v_offset", (float)(cos(glfwGetTime()) / 2.0f));

            va.bind();
            ib.bind();
            GLCALL(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, nullptr));

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

out:
    oogaboogaExit();
    
    return 0;
}
