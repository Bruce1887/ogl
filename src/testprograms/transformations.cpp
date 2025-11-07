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

        VertexBuffer vb(vertices, sizeof(vertices));

        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(3);
        layout.Push<float>(2);

        VertexArray va;
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

        Shader horizontal_shader;
        horizontal_shader.addShader(SHADER_DIR / "horizontal.vert", ShaderType::VERTEX);
        horizontal_shader.addShader(SHADER_DIR / "horizontal.frag", ShaderType::FRAGMENT);
        horizontal_shader.CreateProgram();
        horizontal_shader.Bind();

        Texture texture1((TEXTURE_DIR / "lennart.jpg").string(),0);
        texture1.Bind();
        horizontal_shader.SetUniform("u_texture1", 0);

        Texture texture2((TEXTURE_DIR / "alf.png").string(),1);
        texture2.Bind();	
        horizontal_shader.SetUniform("u_texture2", 1);
        horizontal_shader.Unbind();


		Shader spinning_shader;
		spinning_shader.addShader(SHADER_DIR / "spinning.vert", ShaderType::VERTEX);
        spinning_shader.addShader(SHADER_DIR / "spinning.frag", ShaderType::FRAGMENT);
        spinning_shader.CreateProgram();
		spinning_shader.Bind();
		spinning_shader.SetUniform("u_tex_scale", 3.0f);
	
		texture1.Bind();
		spinning_shader.SetUniform("u_texture1",0);
		texture2.Bind();
		spinning_shader.SetUniform("u_texture2",1);
		spinning_shader.Unbind();

        // Unbind everything
        va.Unbind();
        vb.Unbind();
        ib.Unbind();

        Renderer renderer;
       
        while (!glfwWindowShouldClose(window))
        {
            renderer.Clear();            
			horizontal_shader.Bind();
			horizontal_shader.SetUniform("u_time", (float)glfwGetTime());
            renderer.Draw(va, ib, horizontal_shader);

			spinning_shader.Bind();
			spinning_shader.SetUniform("u_time", (float)glfwGetTime());

			renderer.Draw(va, ib, spinning_shader);

            glfwSwapBuffers(window);

            glfwPollEvents();
        }
    }

out:
    oogaboogaExit();
    
    return 0;
}
