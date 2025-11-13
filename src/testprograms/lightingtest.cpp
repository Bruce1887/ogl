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
        
        // ######## Light source ########
        PhongLightConfig lightConfig{
            .lightPosition = glm::vec3(10.0f, 10.0f, 10.0f),
            .ambientLight = glm::vec3(0.2f, 0.2f, 0.2f),
            .diffuseLight = glm::vec3(1.0f, 1.0f, 1.0f),
            .specularLight = glm::vec3(1.0f, 1.0f, 1.0f)
        };

        VertexArray light_va;
        light_va.bind();
        VertexBuffer light_vb(BOX_VERTICES,BOX_VERTICES_SIZE, &light_va);
        VertexBufferLayout light_layout;
        light_layout.push<float>(3); // position only
        light_va.addBuffer(light_vb, light_layout);   
        
        Scene scene(camera, lightConfig);

        
        VertexArray box_va;
        box_va.bind();
        VertexBuffer box_vb(BOX_VERTICES_NORM_TEX, BOX_VERTICES_NORM_TEX_SIZE, &box_va);
        VertexBufferLayout layout_normals;
        layout_normals.push<float>(3); // position
        layout_normals.push<float>(3); // normal
        layout_normals.push<float>(2); // texture coord
        box_va.addBuffer(box_vb, layout_normals);
        Mesh box_mesh(&box_va, nullptr);

        Shader phongShader;
        phongShader.addShader("PhongTEX.vert", ShaderType::VERTEX);
        phongShader.addShader("PhongTEX.frag", ShaderType::FRAGMENT);
        phongShader.createProgram();
        phongShader.bind();
        phongShader.setUniform("light_ambient", glm::vec3(0.2f, 0.2f, 0.2f));

        MeshRenderable box_renderable(&box_mesh, &phongShader);
        box_renderable.setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)));
        
        Texture box_tex((TEXTURE_DIR / "container.jpg").string(), 0);
        box_tex.targetUniform = "u_texture";
        box_renderable.m_textureReferences = std::vector<Texture *>{&box_tex};

        scene.addRenderable(&box_renderable);        

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