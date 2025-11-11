#include <iostream>

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

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (oogaboogaInit(__FILE__))
        goto out;

    {
        VertexArray va;
        va.bind();

        constexpr const float* vertices = plane_vertices;
        constexpr const unsigned int* indices = plane_indices;
        VertexBuffer vb(vertices, 4 * 5 * sizeof(float));

        VertexBufferLayout layout;
        layout.push<float>(3);
        layout.push<float>(2);

        va.addBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

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
            GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            shader.bind();

            MovementInput movementInput = getUserMovementInput(window);
            cameraOrbitControl(camera, movementInput, deltaTime);
            glm::mat4 model_matrix = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            model_matrix = glm::translate(model_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
            model_matrix = glm::rotate(model_matrix, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 1.0));

            shader.bind();
            shader.setUniform("model", model_matrix);
            shader.setUniform("view", camera.GetViewMatrix());
            shader.setUniform("projection", camera.GetProjectionMatrix());

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