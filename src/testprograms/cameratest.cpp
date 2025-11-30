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

bool cameraFunctionMode = true; // true = fly, false = orbit

int main(int, char **)
{
    // Initialise GLAD and GLFW
    if (oogaboogaInit(__FILE__))
        goto out;

    // set key-callback (for movement input)
    glfwSetKeyCallback(g_window, [](GLFWwindow *wdw, int key, int /*scancode*/, int action, int mods)
                       {        
        if (key == GLFW_KEY_W && mods & GLFW_MOD_CONTROL)
            glfwSetWindowShouldClose(wdw, GLFW_TRUE); 
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
            std::cout << "Toggling camera function mode. Now in " << (cameraFunctionMode ? "orbit" : "fly") << " mode." << std::endl;
            cameraFunctionMode = !cameraFunctionMode;
        }

        g_InputManager->movementInput.updateMovement(key, action, mods); });
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
        auto lightBox_VA_ptr = std::make_unique<VertexArray>();

        lightBox_VA_ptr->bind();
        auto lightBox_VB_ptr = std::make_unique<VertexBuffer>(BOX_VERTICES, BOX_VERTICES_SIZE, lightBox_VA_ptr.get());
        VertexBufferLayout lightBox_layout;
        lightBox_layout.push<float>(3); // position
        lightBox_VA_ptr->addBuffer(lightBox_VB_ptr.get(), lightBox_layout);
        auto lightBox_IBO_ptr = std::make_unique<IndexBuffer>(BOX_INDICES, BOX_INDICES_COUNT);
        
        auto lightBox_mesh_ptr = std::make_shared<Mesh>(std::move(lightBox_VA_ptr), std::move(lightBox_VB_ptr), std::move(lightBox_IBO_ptr));
        Shader lightBox_shader;
        lightBox_shader.addShader("3D.vert", ShaderType::VERTEX);
        lightBox_shader.addShader("constColor.frag", ShaderType::FRAGMENT);
        lightBox_shader.createProgram();
        lightBox_shader.bind();
        lightBox_shader.setUniform("u_color", lightSource.config.diffuseLight); // Set the box color to the light's diffuse color

        auto lightBox_shader_ptr = std::make_shared<Shader>(lightBox_shader);
        MeshRenderable lightBox_renderable(lightBox_mesh_ptr, lightBox_shader_ptr);
        lightSource.visualRepresentation = &lightBox_renderable;

        // create the scene
        Scene scene(camera, lightSource);

        // Create the main object (a big box)
        auto box_va_ptr = std::make_unique<VertexArray>();
        box_va_ptr->bind();
        auto box_VB_ptr = std::make_unique<VertexBuffer>(BOX_VERTICES_NORM_TEX, BOX_VERTICES_NORM_TEX_SIZE, box_va_ptr.get());
        VertexBufferLayout layout_normals;
        layout_normals.push<float>(3); // position
        layout_normals.push<float>(3); // normal
        layout_normals.push<float>(2); // texture coord
        box_va_ptr->addBuffer(box_VB_ptr.get(), layout_normals);

        auto box_mesh_ptr = std::make_shared<Mesh>(std::move(box_va_ptr), std::move(box_VB_ptr));
            
        Shader phongShader;
        phongShader.addShader("3DLighting_Tex.vert", ShaderType::VERTEX);
        phongShader.addShader("PhongTEX.frag", ShaderType::FRAGMENT);
        phongShader.createProgram();

        auto phongShader_ptr = std::make_shared<Shader>(phongShader);        
        MeshRenderable box_renderable(box_mesh_ptr, phongShader_ptr);
        
        box_renderable.setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)));
        
        auto box_tex = std::make_shared<Texture>((TEXTURE_DIR / "container.jpg").string(), 0);
        box_tex->targetUniform = "u_texture";
        box_renderable.m_textureReferences = std::vector<std::shared_ptr<Texture>>{box_tex};

        scene.addRenderable(&box_renderable);

        FrameTimer frameTimer;

        while (!glfwWindowShouldClose(g_window))
        {
            scene.tick();

            float slowedTime = frameTimer.getCurrentTime() * 0.5f;
            scene.m_lightSource.config.lightPosition = glm::vec3(15.0f * sinf(slowedTime), 15.0f * sinf(slowedTime), 15.0f * cosf(slowedTime));

            if (cameraFunctionMode)
                scene.m_activeCamera.flyControl(g_InputManager, frameTimer.getDeltaTime());
            else
                scene.m_activeCamera.orbitControl(g_InputManager, frameTimer.getDeltaTime());

            scene.renderScene();
        }
    }

out:
    oogaboogaExit();

    return 0;
}