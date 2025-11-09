#include <iostream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Common.h"
#include "Camera.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>

void CameraOrbitControl(Camera &camera, float deltaTime)
{
    int FORWARD = 0;
    int RIGHT = 0;
    bool SHIFT = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        FORWARD += 1;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        FORWARD -= 1;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        RIGHT -= 1;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        RIGHT += 1;

    float speed = 5.0f * deltaTime;
    if (SHIFT)
        speed *= 5.0f;


    glm::vec3 offset = camera.m_Position - camera.m_Target;
    float distance = glm::length(offset);

    if (RIGHT != 0)
    {
        float angle = RIGHT * speed;
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
        offset = glm::vec3(rotation * glm::vec4(offset, 1.0f));
    }

    if (FORWARD != 0)
    {
        distance -= FORWARD * speed * 3.0f;
        distance = glm::max(distance, 1.0f); // Don't get too close
        offset = glm::normalize(offset) * distance;
    }

    camera.m_Position = camera.m_Target + offset;
}

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (oogaboogaInit(__FILE__))
        goto out;

    {
        float box_vertices[] = {
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

        unsigned int box_indices[] = {
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

        VertexArray box_va;
        box_va.Bind();

        VertexBuffer box_vb(box_vertices, sizeof(box_vertices));

        VertexBufferLayout box_layout;
        box_layout.Push<float>(3);
        box_layout.Push<float>(2);

        box_va.AddBuffer(box_vb, box_layout);

        IndexBuffer box_ib(box_indices, sizeof(box_indices) / sizeof(unsigned int));

        Shader shader;
        shader.addShader((SHADER_DIR / "3D.vert").string(), ShaderType::VERTEX);
        shader.addShader((SHADER_DIR / "3D.frag").string(), ShaderType::FRAGMENT);
        shader.CreateProgram();

        Texture texture1((TEXTURE_DIR / "container.jpg").string(), 0);
        texture1.Bind();

        Texture texture2((TEXTURE_DIR / "cowday.png").string(), 1);
        texture2.Bind();

        shader.Bind();
        shader.SetUniform("u_texture1", 0);
        shader.SetUniform("u_texture2", 1);

        CameraConfiguration cam_config{
            .fov = 45.0f,
            .aspect = (float)window_X / (float)window_Y,
            .near = 0.1f,
            .far = 100.0f};

        Camera camera(cam_config);
        camera.m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
        camera.m_Position = glm::vec3(0.0f, 0.0f, 15.0f);

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;

        while (!glfwWindowShouldClose(window))
        {
            Renderer::Clear();

            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            shader.Bind();

            CameraOrbitControl(camera, deltaTime);
            glm::mat4 model_matrix = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
            model_matrix = glm::rotate(model_matrix, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 1.0));

            shader.Bind();
            shader.SetUniform("model", model_matrix);
            shader.SetUniform("view", camera.GetViewMatrix());
            shader.SetUniform("projection", camera.GetProjectionMatrix());

            Renderer::Draw(box_va, box_ib, shader);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

out:
    oogaboogaExit();

    return 0;
}