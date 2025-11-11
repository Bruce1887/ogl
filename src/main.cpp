#include <iostream>

// #include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Common.h"
#include "Camera.h"
#include "Scene.h"

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
        CameraConfiguration cam_config{
            .fov = 45.0f,
            .aspect = (float)window_X / (float)window_Y,
            .near = 0.1f,
            .far = 100.0f};

        Camera camera(cam_config);
        camera.m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
        camera.m_Position = glm::vec3(0.0f, 0.0f, 15.0f);

        Scene scene(camera);

        VertexBufferLayout layout;
        layout.push<float>(3);
        layout.push<float>(2);

        // ######## BOX ########
        
        VertexArray box_va;
        box_va.bind();

        VertexBuffer box_vb(box_vertices, sizeof(box_vertices));

        box_va.addBuffer(box_vb, layout);

        IndexBuffer box_ib(box_indices, sizeof(box_indices) / sizeof(unsigned int));

        Shader shader;
        shader.addShader((SHADER_DIR / "3D.vert").string(), ShaderType::VERTEX);
        shader.addShader((SHADER_DIR / "3D.frag").string(), ShaderType::FRAGMENT);
        shader.createProgram();

        Texture box_tex_1((TEXTURE_DIR / "container.jpg").string(), 0);
        box_tex_1.bind();

        Texture box_tex_2((TEXTURE_DIR / "cowday.png").string(), 1);
        box_tex_2.bind();

        shader.bind();
        shader.setUniform("u_texture1", 0);
        shader.setUniform("u_texture2", 1);

        Mesh box_mesh(&box_va, &box_ib);
        MeshRenderable box_renderable1(&box_mesh, &shader);
        scene.addRenderable(&box_renderable1);

        MeshRenderable box_renderable2(&box_mesh, &shader);
        box_renderable2.setPosition(glm::vec3(2.0f, 1.0f, 1.0f));

        scene.addRenderable(&box_renderable2);
        
        // ######## PLANE ########

        VertexArray plane_va;
        plane_va.bind();
        VertexBuffer plane_vb(plane_vertices, 4 * 5 * sizeof(float));
        plane_va.addBuffer(plane_vb, layout);
        IndexBuffer plane_ib(plane_indices, 6);

        Shader shader2;
        shader2.addShader((SHADER_DIR / "3D.vert").string(), ShaderType::VERTEX);
        shader2.addShader((SHADER_DIR / "3D.frag").string(), ShaderType::FRAGMENT);
        shader2.createProgram();

        Texture plane_tex_1((TEXTURE_DIR / "grass.jpg").string(), 3);
        plane_tex_1.bind();

        Texture plane_tex_2((TEXTURE_DIR / "alf.png").string(), 4);
        plane_tex_2.bind();

        shader2.bind();
        shader2.setUniform("u_texture1", 3);
        shader2.setUniform("u_texture2", 4);

        Mesh plane_mesh(&plane_va, &plane_ib);
        MeshRenderable plane_renderable(&plane_mesh, &shader2);
        plane_renderable.setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, 10.0f)));
        plane_renderable.setTransform(
            glm::rotate(
                glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, 10.0f)),
                glm::radians(90.0f),
                glm::vec3(1.0f, 0.0f, 1.0f)));
        scene.addRenderable(&plane_renderable);

        // #######################

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;

        while (!glfwWindowShouldClose(window))
        {
            GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            MovementInput movementInput = getUserMovementInput(window);
            cameraOrbitControl(scene.m_activeCamera, movementInput, deltaTime);

            scene.renderScene();
        }
    }

out:
    oogaboogaExit();

    return 0;
}