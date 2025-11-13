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
            .near = 1.0f,
            .far = 300.0f};

        Camera camera(cam_config);
        camera.m_Position = glm::vec3(0.0f, 20.0f, 35.0f);
        camera.m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
            
        PhongLightConfig lightConfig{
            .lightPosition = glm::vec3(10.0f, 10.0f, 10.0f),
            .ambientLight = glm::vec3(0.2f, 0.2f, 0.2f),
            .diffuseLight = glm::vec3(1.0f, 1.0f, 1.0f),
            .specularLight = glm::vec3(1.0f, 1.0f, 1.0f)
        };

        Scene scene(camera, lightConfig);

        // ######## SHARED ########
        // Layout and shader is shared between these two renderables (box and plane)
        VertexBufferLayout layout;
        layout.push<float>(3);
        layout.push<float>(2);

        Shader shader;
        shader.addShader("3D_TEX.vert", ShaderType::VERTEX);
        shader.addShader("2TEX_AB.frag", ShaderType::FRAGMENT);
        shader.createProgram();
        shader.bind();
        shader.setUniform("u_texture1", 0);
        shader.setUniform("u_texture2", 1);

        // ######## BOX ########
        VertexArray box_va;
        box_va.bind();
        VertexBuffer box_vb(BOX_VERTICES_TEX, BOX_VERTICES_TEX_SIZE, &box_va);
        box_va.addBuffer(box_vb, layout);
        IndexBuffer box_ib(BOX_INDICES_TEX, BOX_INDICES_TEX_COUNT);

        Texture box_tex_1((TEXTURE_DIR / "container.jpg").string(), 0);
        box_tex_1.targetUniform = "u_texture1";
        Texture box_tex_2((TEXTURE_DIR / "cowday.png").string(), 1);
        box_tex_2.targetUniform = "u_texture2";

        // Create mesh (shared data)
        Mesh box_mesh(&box_va, &box_ib);

        // Create renderable (per-instance data, has reference to shared mesh and shader)
        MeshRenderable box_renderable1(&box_mesh, &shader);
        box_renderable1.setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)));
        box_renderable1.m_textureReferences = std::vector<Texture *>{&box_tex_1, &box_tex_2};

        scene.addRenderable(&box_renderable1);

        /*
        // A second box renderable to test multiple renderables
        MeshRenderable box_renderable2(&box_mesh, &shader);
        box_renderable2.setPosition(glm::vec3(2.0f, 1.0f, 1.0f));
        box_renderable2.m_textureReferences = std::vector<Texture *>{&box_tex_1, &box_tex_2};

        scene.addRenderable(&box_renderable2);
        */

        // ######## PLANE ########

        VertexArray plane_va;
        plane_va.bind();
        VertexBuffer plane_vb(PLANE_VERTICES_TEX, PLANE_VERTICES_TEX_SIZE, &plane_va);
        plane_va.addBuffer(plane_vb, layout);
        IndexBuffer plane_ib(PLANE_INDICES, PLANE_INDICES_COUNT);

        Texture plane_tex_1((TEXTURE_DIR / "grass.jpg").string(), 0);
        plane_tex_1.targetUniform = "u_texture1";
        Texture plane_tex_2((TEXTURE_DIR / "alf.png").string(), 1);
        plane_tex_2.targetUniform = "u_texture2";

        Mesh plane_mesh(&plane_va, &plane_ib);
        MeshRenderable plane_renderable(&plane_mesh, &shader);
        plane_renderable.setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 5.0f)));
        plane_renderable.m_textureReferences = std::vector<Texture *>{&plane_tex_1, &plane_tex_2};
        // plane_renderable.m_textureReferences = std::vector<Texture *>{&box_tex_1, &box_tex_2};

        scene.addRenderable(&plane_renderable);

        // #######################
        FrameTimer frameTimer;

        while (!glfwWindowShouldClose(window))
        {
            scene.tick();

            MovementInput movementInput = getUserMovementInput(window);
            scene.m_activeCamera.orbitControl(movementInput, frameTimer.getDeltaTime());

            scene.renderScene();
        }
    }

out:
    oogaboogaExit();

    return 0;
}