#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
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
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *monitor = nullptr;

    // GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    GLsizei window_X = 640;
    GLsizei window_Y = 480;

    GLFWwindow *window = glfwCreateWindow(window_X, window_Y, "Lennart Jähkel", monitor, NULL);
    if (!window)
    {
        const char **e_msg = nullptr;
        glfwGetError(e_msg);
        std::cerr << "Failed to create window: " << e_msg << std::endl;

        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

    // set window-resize-callback (resize viewport)
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow * /*window*/, int width, int height)
                                   {
        glViewport(0, 0, width, height); });

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
        
        IndexBuffer ib(indices, 6);

        Shader shader("resources/shaders/basic.shader");
        shader.Bind();
        shader.SetUniform4f("u_color", {0.0f, 0.3f, 0.8f, 1.0f});

        Texture texture("resources/textures/lennart.jpg");
        texture.Bind();
        shader.SetUniform1i("u_texture", 0);

        // Unbind everything
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        Renderer renderer;

        glfwSetKeyCallback(window, [](GLFWwindow *wdw, int key, int /*scancode*/, int action, int mods)
                           {
            std::cout << "key: " << key << ", action: " << action  << ", mods:" << mods << std::endl;
            if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
                glfwSetWindowShouldClose(wdw, GLFW_TRUE); });


        GLCALL(glClearColor(0.2f, 0.1f, 0.2f, 1.0f)); // dark purple bg color


        float red = 0.0f;
        float increment = 0.01f;
        while (!glfwWindowShouldClose(window))
        {
            renderer.Clear();


            shader.Bind();
            shader.SetUniform4f("u_color", {red, 0.3f, 0.8f, 1.0f});

            
            renderer.Draw(va, ib, shader);

            red = std::fmod(red + increment, 10.0f);

            glfwSwapBuffers(window);

            glfwPollEvents();
        }
    }
    glfwTerminate();
    return 0;
}
