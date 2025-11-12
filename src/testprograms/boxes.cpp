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

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (oogaboogaInit(__FILE__))
        goto out;

    {
        /* If we define the full cube with shared vertices, texture mapping gets messed up
        float vertices[] = {
            // positions          // texture coords
            // front vertices
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [0] front top right
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [1] front bottom right
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // [2] front bottom left
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  // [3] front top left

            // back vertices
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // [4] back top right
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // [5] back bottom right
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [6] back bottom left
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f   // [7] back top left
        };
        unsigned int indices[] = {
            // Front face
            0, 1, 2,
            2, 3, 0,
            // Back face
            4, 5, 6,
            6, 7, 4,
            // Left face
            3, 2, 6,
            6, 7, 3,
            // Right face
            0, 1, 5,
            5, 4, 0,
            // Top face
            0, 3, 7,
            7, 4, 0,
            // Bottom face
            1, 2, 6,
            6, 5, 1,
        };
        */

        // Define cube with unique vertices per face for correct texture mapping
        float vertices[] = {
            // Front face (z = 0.5)
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // [0]
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [1]
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [2]
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  // [3]

            // Back face (z = -0.5)
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [4]
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // [5]
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // [6]
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [7]

            // Left face (x = -0.5)
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [8]
            -0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [9]
            -0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [10]
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [11]

            // Right face (x = 0.5)
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [12]
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [13]
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [14]
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [15]

            // Top face (y = 0.5)
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, // [16]
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,  // [17]
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // [18]
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // [19]

            // Bottom face (y = -0.5)
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // [20]
            -0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // [21]
            0.5f, -0.5f, 0.5f, 1.0f, 1.0f,   // [22]
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f   // [23]
        };

        unsigned int indices[] = {
            // Front face
            0, 1, 2,
            2, 3, 0,
            // Back face
            4, 5, 6,
            6, 7, 4,
            // Left face
            8, 9, 10,
            10, 11, 8,
            // Right face
            12, 13, 14,
            14, 15, 12,
            // Top face
            16, 17, 18,
            18, 19, 16,
            // Bottom face
            20, 21, 22,
            22, 23, 20};
            
        VertexArray va;
        va.bind();

        VertexBuffer vb(vertices, sizeof(vertices));

        VertexBufferLayout layout;
        layout.push<float>(3);
        layout.push<float>(2);

        va.addBuffer(vb, layout);

        IndexBuffer ib(indices, sizeof(indices) / sizeof(unsigned int));

        Shader shader;
        shader.addShader((SHADER_DIR / "3D.vert").string(), ShaderType::VERTEX);
        shader.addShader((SHADER_DIR / "3D.frag").string(), ShaderType::FRAGMENT);
        shader.createProgram();

        Texture texture1((TEXTURE_DIR / "container.jpg").string(), 0);
        texture1.bind();

        Texture texture2((TEXTURE_DIR / "cowday.png").string(), 1);
        texture2.bind();

        shader.bind();
        shader.setUniform("u_texture1", 0);
        shader.setUniform("u_texture2", 1);

        std::vector<glm::vec3> cubePositions = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(2.0f, 5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f, 3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f, 2.0f, -2.5f),
            glm::vec3(1.5f, 0.2f, -1.5f),
            glm::vec3(-1.3f, 1.0f, -1.5f)};

        float fov = 45.0f;

        struct settings
        {
            float fov;
            float aspect;
        };

        settings w_settings{fov, (float)window_X / (float)window_Y};

        glfwSetWindowUserPointer(window, &w_settings);

        glfwSetKeyCallback(window, [](GLFWwindow *wdw, int key, int /* scancode */, int action, int mods)
                           {
            if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
                glfwSetWindowShouldClose(wdw, true);
            else {
                settings *w_settings_ref = (settings *)glfwGetWindowUserPointer(wdw);
                std::cout << "[fov,aspect] : [" << w_settings_ref->fov << ", " << w_settings_ref->aspect << "]" << std::endl;
                if (key == GLFW_KEY_K) w_settings_ref->fov -= 1.0f;
                else if (key == GLFW_KEY_L) w_settings_ref->fov += 1.0f;
                else if (key == GLFW_KEY_N && action == GLFW_PRESS) w_settings_ref->aspect -= 0.3f;
                else if (key == GLFW_KEY_M && action == GLFW_PRESS) w_settings_ref->aspect += 0.3f;
            } });

        std::cout << "Use K/L to decrease/increase FOV" << std::endl;
        std::cout << "Use N/M to decrease/increase Aspect Ratio" << std::endl;

        while (!glfwWindowShouldClose(window))
        {
            GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            shader.bind();

            float frameTime = static_cast<float>(glfwGetTime());

            glm::mat4 viewMatrix = glm::mat4(1.0f);
            glm::vec3 cameraPos = glm::vec3(cos(frameTime * 0.3), 0.0f, sin(frameTime * 0.3)) * 15.0f;
            viewMatrix = glm::translate(viewMatrix, cameraPos);
            viewMatrix = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat4 projectionMatrix = glm::mat4(1.0f);

            // set view and projection matrices (camera)
            projectionMatrix = glm::perspective(glm::radians(w_settings.fov), w_settings.aspect, 0.1f, 100.0f);
            shader.bind();
            shader.setUniform("view", viewMatrix);
            shader.setUniform("projection", projectionMatrix);

            // set model matrix and draw cubes
            for (size_t i = 0; i < cubePositions.size(); i++)
            {
                glm::mat4 modelMatrix = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
                modelMatrix = glm::translate(modelMatrix, cubePositions[i]);
                if (i % 3 == 0)
                    modelMatrix = glm::rotate(modelMatrix, frameTime * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

                shader.setUniform("model", modelMatrix);

                va.bind();
                ib.bind();
                GLCALL(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, nullptr));
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

out:
    oogaboogaExit();

    return 0;
}
