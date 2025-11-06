#include "Common.h"
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


int main(int, char **)
{
    // Initialise GLAD and GLFW
    if(oogaboogaInit(__FILE__)) goto out;
    
    {
        float vertices[] = {
            -0.5f,-0.5f,    1.0f,0.0f,0.0f,     0.0f,0.0f, // bottom left
            0.5f,-0.5f,     0.0f,1.0f,0.0f,     1.0f,0.0f, // bottom right
            0.5f,0.5f,      0.0f,0.0f,1.0f,     1.0f,1.0f, // top right
            -0.5f,0.5f,     1.0f,1.0f,1.0f,     0.0f,1.0f, // top left
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        GLCALL(glEnable(GL_BLEND));
        
        GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        VertexBuffer vb(vertices, sizeof(vertices));

        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(3);
        layout.Push<float>(2);

        VertexArray va;
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

        Shader shader;
        shader.addShader(SHADER_DIR / "basic.vert", ShaderType::VERTEX);
        shader.addShader(SHADER_DIR / "basic.frag", ShaderType::FRAGMENT);
        shader.CreateProgram();

        shader.Bind();
        // shader.SetUniform4f("u_color", {0.0f, 0.3f, 0.8f, 1.0f});

        Texture texture1((TEXTURE_DIR / "lennart.jpg").string(),0);
        texture1.Bind();
        shader.SetUniform1i("u_texture1", 0);
        
        Texture texture2((TEXTURE_DIR / "cowday.png").string(),1);
        texture2.Bind();
        shader.SetUniform1i("u_texture2", 1);

        // Unbind everything    
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        Renderer renderer;

        
        while (!glfwWindowShouldClose(window))
        {
            renderer.Clear();            
            
            renderer.Draw(va, ib, shader);

            glfwSwapBuffers(window);

            glfwPollEvents();
        }
    }

out:
    oogaboogaExit();
    
    return 0;
}
