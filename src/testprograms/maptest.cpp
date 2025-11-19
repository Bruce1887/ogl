#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "MeshRenderable.h"
#include "ObjLoader.h"
#include "Frametimer.h"

// Simple free-fly camera controller (WASD + Shift for speed, Q/E for vertical)
static void updateCameraFreeFly(Camera &camera, MovementInput movementInput, float deltaTime)
{
    float baseSpeed = 10.0f;
    if (movementInput.shiftDown)
        baseSpeed *= 4.0f;
    float speed = baseSpeed * deltaTime;

    // Derive forward/right from current look direction
    glm::vec3 forwardDir = glm::normalize(camera.m_Target - camera.m_Position);
    glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, camera.m_Up));

    camera.m_Position += forwardDir * (float)movementInput.wasd.forward * speed;
    camera.m_Position += rightDir * (float)movementInput.wasd.right * speed;

    // Keep target in front at fixed distance to preserve forward direction (like FPS style)
    float targetDistance = 50.0f; // arbitrary large so we look far ahead
    camera.m_Target = camera.m_Position + forwardDir * targetDistance;
}

int main(int, char **)
{
    if (oogaboogaInit("maptest"))
        return -1;

    // Camera configuration suitable for a larger map
    CameraConfiguration camConfig{ .fov = 60.0f, .aspect = (float)window_X / (float)window_Y, .near = 0.1f, .far = 1000.0f };
    Camera camera(camConfig);
    camera.m_Position = glm::vec3(0.0f, 5.0f, 10.0f);
    camera.m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

    PhongLightConfig lightCfg{
        .lightPosition = glm::vec3(100.0f, 200.0f, 100.0f),
        .ambientLight = glm::vec3(0.3f, 0.3f, 0.3f),
        .diffuseLight = glm::vec3(1.0f, 1.0f, 1.0f),
        .specularLight = glm::vec3(0.8f, 0.8f, 0.8f)
    };
    LightSource lightSource{ .config = lightCfg, .visualRepresentation = nullptr };
    Scene scene(camera, lightSource);

    // Shader for vertex colors (not textures)
    Shader shader;
    shader.addShader("3DLighting_VertColor.vert", ShaderType::VERTEX); 
    shader.addShader("PhongVertColor.frag", ShaderType::FRAGMENT);
    shader.createProgram();
    shader.bind();

    // Load the map OBJ with materials
    fs::path mapPath = fs::path("resources") / "map" / "testMap.obj";
    auto meshes = ObjLoader::LoadOBJWithMaterials(mapPath);
    if (meshes.empty())
    {
        std::cerr << "[maptest] Failed to load map OBJ at " << mapPath << std::endl;
        oogaboogaExit();
        return -2;
    }

    // Create renderables for each material
    std::vector<MeshRenderable*> renderables;
    for (auto &pair : meshes)
    {
        const std::string &matName = pair.first;
        Mesh *mesh = pair.second;
        
        auto *renderable = new MeshRenderable(mesh, &shader);
        renderable->setTransform(glm::mat4(1.0f));
        scene.addRenderable(renderable);
        renderables.push_back(renderable);
        
        // std::cout << "[maptest] Added material: " << matName << std::endl;
    }

    FrameTimer frameTimer;
    while (!glfwWindowShouldClose(g_window))
    {
        scene.tick();
        float dt = frameTimer.getDeltaTime();
        MovementInput movementInput = getUserMovementInput(g_window);
        updateCameraFreeFly(scene.m_activeCamera, movementInput, dt);
        scene.renderScene();
    }

    // Cleanup (Mesh resources leak intentionally for simplicity here; integrate smart pointers later if needed)
    oogaboogaExit();
    return 0;
}
