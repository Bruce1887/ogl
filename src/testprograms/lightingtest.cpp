#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

#include "Shader.h"
#include "Texture.h"
#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Frametimer.h"
#include "MeshRenderable.h"

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
        // ######## Camera setup ########
        CameraConfiguration cam_config{
            .fov = 45.0f,
            .aspect = (float)window_X / (float)window_Y,
            .near = 1.0f,
            .far = 300.0f};

        Camera camera(cam_config);
        camera.m_Position = glm::vec3(0.0f, 20.0f, 35.0f);
        camera.m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

        // ######## Light source ########
        PhongLightConfig lightConfig{
            .lightPosition = glm::vec3(15.0f, 10.0f, 10.0f),
            .ambientLight = glm::vec3(0.2f, 0.2f, 0.2f),
            .diffuseLight = glm::vec3(1.0f, 1.0f, 0.7f),
            .specularLight = glm::vec3(1.0f, 1.0f, 1.0f)};
        LightSource lightSource{
            .config = lightConfig,
            .visualRepresentation = nullptr};

        // Create a small box to visualize the light source
        VertexArray lightBox_VA;
        lightBox_VA.bind();
        VertexBuffer lightBox_VB(BOX_VERTICES, BOX_VERTICES_SIZE, &lightBox_VA);
        VertexBufferLayout lightBox_layout;
        lightBox_layout.push<float>(3); // position
        lightBox_VA.addBuffer(lightBox_VB, lightBox_layout);
        IndexBuffer lightBox_IBO(BOX_INDICES, BOX_INDICES_COUNT);
        Mesh lightBox_mesh(&lightBox_VA, &lightBox_IBO);
        Shader lightBox_shader;
        lightBox_shader.addShader("3D.vert", ShaderType::VERTEX);
        lightBox_shader.addShader("constColor.frag", ShaderType::FRAGMENT);
        lightBox_shader.createProgram();
        lightBox_shader.bind();
        lightBox_shader.setUniform("u_color", lightSource.config.diffuseLight); // Set the box color to the light's diffuse color
        MeshRenderable lightBox_renderable(&lightBox_mesh, &lightBox_shader);
        lightSource.visualRepresentation = &lightBox_renderable;

        // create the scene
        Scene scene(camera, lightSource);

        // Create the main object (a big box)
        VertexArray box_VA;
        box_VA.bind();
        VertexBuffer box_VB(BOX_VERTICES_NORM_TEX, BOX_VERTICES_NORM_TEX_SIZE, &box_VA);
        VertexBufferLayout layout_normals;
        layout_normals.push<float>(3); // position
        layout_normals.push<float>(3); // normal
        layout_normals.push<float>(2); // texture coord
        box_VA.addBuffer(box_VB, layout_normals);
        Mesh box_mesh(&box_VA, nullptr);

        Shader phongShader;
        phongShader.addShader("3DLighting_Tex.vert", ShaderType::VERTEX);
        phongShader.addShader("PhongTEX.frag", ShaderType::FRAGMENT);
        phongShader.createProgram();

        MeshRenderable box_renderable(&box_mesh, &phongShader);
        box_renderable.setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)));

        Texture box_tex((TEXTURE_DIR / "container.jpg").string(), 0);
        box_tex.targetUniform = "u_texture";
        box_renderable.m_textureReferences = std::vector<Texture *>{&box_tex};

        scene.addRenderable(&box_renderable);

        FrameTimer frameTimer;

        while (!glfwWindowShouldClose(g_window))
        {
            scene.tick();

            float slowedTime = frameTimer.getCurrentTime() * 0.5f;
            scene.m_lightSource.config.lightPosition = glm::vec3(15.0f * sinf(slowedTime), 15.0f * sinf(slowedTime), 15.0f * cosf(slowedTime));

            scene.m_activeCamera.orbitControl(g_InputManager, frameTimer.getDeltaTime());

            double deltaX, deltaY;
            if (g_InputManager->mouseMoveInput.fetchDeltas(deltaX, deltaY))
            {
                // std::cout << "Mouse moved: deltaX=" << deltaX << ", deltaY=" << deltaY << std::endl;
            }

            scene.renderScene();
        }
    }

out:
    oogaboogaExit();

    return 0;
}