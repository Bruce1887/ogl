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
    if(oogaboogaInit(__FILE__)) goto out;
    
    {
        float vertices[] = {
            -0.5f,-0.5f,0.0f,0.0f, // bottom left
            0.5f,-0.5f,1.0f,0.0f, // bottom right
            0.5f,0.5f,1.0f,1.0f, // top right
            -0.5f,0.5f,0.0f,1.0f, // top left
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0};   

        GLCALL(glEnable(GL_BLEND));
        
        GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        VertexBuffer vb(vertices, sizeof(vertices));

        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(2);

        VertexArray va;
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6,BufferUsage::STATIC_DRAW);

        Shader shader;
        shader.addShader(SHADER_DIR / "basic.vert", ShaderType::VERTEX);
        shader.addShader(SHADER_DIR / "basic.frag", ShaderType::FRAGMENT);
        shader.CreateProgram();

        shader.Bind();
        shader.SetUniform4f("u_color", {0.0f, 0.3f, 0.8f, 1.0f});

        Texture texture((TEXTURE_DIR / "lennart.jpg").string());
        texture.Bind();
        shader.SetUniform1i("u_texture", 0);

        // Unbind everything
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        Renderer renderer;

        float green = 0;
        
        while (!glfwWindowShouldClose(window))
        {
            renderer.Clear();

            shader.Bind(); // we dont need to bind shader for rendering purposes, but for updating the uniforms.
            green = (sin(glfwGetTime()) / 2.0f) + 0.5f;
            shader.SetUniform4f("u_color", {0.0, green, 0.0f, 1.0f});

            renderer.Draw(va, ib, shader);

            glfwSwapBuffers(window);

            glfwPollEvents();
        }
    }

    glfwTerminate(); // dont call glfwTerminate() if glfw failed to initialize
out:
    oogaboogaExit();
    
    return 0;
}
