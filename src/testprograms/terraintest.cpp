#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "MeshRenderable.h"
#include "TerrainGenerator.h"
#include "Frametimer.h"
#include <iomanip>

// Free-fly camera controller (WASD + Mouse, Shift for speed, Q/E for vertical)
static void updateCameraFreeFly(Camera &camera, MovementInput movementInput, float deltaTime)
{
    float baseSpeed = 30.0f;
    if (movementInput.shiftDown)
        baseSpeed *= 3.0f;
    float speed = baseSpeed * deltaTime;

    // Derive forward/right from current look direction
    glm::vec3 forwardDir = glm::normalize(camera.m_Target - camera.m_Position);
    glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, camera.m_Up));

    // WASD movement
    camera.m_Position += forwardDir * (float)movementInput.wasd.forward * speed;
    camera.m_Position += rightDir * (float)movementInput.wasd.right * speed;

    // Q/E for vertical movement
    if (movementInput.qDown)
        camera.m_Position += glm::vec3(0.0f, -1.0f, 0.0f) * speed;
    if (movementInput.eDown)
        camera.m_Position += glm::vec3(0.0f, 1.0f, 0.0f) * speed;

    // Keep target in front at fixed distance (FPS style)
    float targetDistance = 50.0f;
    camera.m_Target = camera.m_Position + forwardDir * targetDistance;
}

static void handleMouseLook(Camera &camera, double xoffset, double yoffset)
{
    // Mouse sensitivity
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Calculate current direction
    glm::vec3 direction = glm::normalize(camera.m_Target - camera.m_Position);
    
    // Convert to spherical coordinates
    float radius = glm::length(camera.m_Target - camera.m_Position);
    float yaw = glm::degrees(atan2(direction.z, direction.x));
    float pitch = glm::degrees(asin(direction.y));

    // Update angles
    yaw += xoffset;
    pitch -= yoffset;

    // Clamp pitch
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Convert back to cartesian
    glm::vec3 newDirection;
    newDirection.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    newDirection.y = sin(glm::radians(pitch));
    newDirection.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

    camera.m_Target = camera.m_Position + glm::normalize(newDirection) * radius;
}

// Mouse callback
static double lastX = 400, lastY = 300;
static bool firstMouse = true;
static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = ypos - lastY;
    lastX = xpos;
    lastY = ypos;

    Camera* camera = (Camera*)glfwGetWindowUserPointer(window);
    if (camera)
        handleMouseLook(*camera, xoffset, yoffset);
}

int main(int, char **)
{
    if (oogaboogaInit("Terrain Test - Perlin Noise"))
        return -1;

    // Setup mouse input
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Camera configuration for large terrain
    CameraConfiguration camConfig{ 
        .fov = 60.0f, 
        .aspect = (float)window_X / (float)window_Y, 
        .near = 0.1f, 
        .far = 2000.0f 
    };
    Camera camera(camConfig);
    camera.m_Position = glm::vec3(100.0f, 80.0f, 100.0f);  // Further back and higher to see massive mountain
    camera.m_Target = glm::vec3(-50.0f, 60.0f, -50.0f);     // Look towards the mountain range
    camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Set camera as user pointer for mouse callback
    glfwSetWindowUserPointer(g_window, &camera);
    glfwSetCursorPosCallback(g_window, mouse_callback);

    // Light configuration - sun-like light from above
    PhongLightConfig lightCfg{
        .lightPosition = glm::vec3(200.0f, 300.0f, 200.0f),
        .ambientLight = glm::vec3(0.4f, 0.4f, 0.45f),
        .diffuseLight = glm::vec3(0.9f, 0.9f, 0.8f),
        .specularLight = glm::vec3(0.5f, 0.5f, 0.5f)
    };
    LightSource lightSource{ .config = lightCfg, .visualRepresentation = nullptr };
    Scene scene(camera, lightSource);

    // Shader for terrain
    Shader terrainShader;
    terrainShader.addShader("Terrain.vert", ShaderType::VERTEX);
    terrainShader.addShader("TerrainBlend.frag", ShaderType::FRAGMENT);
    terrainShader.createProgram();

    // Load textures
    Texture groundTexture((TEXTURE_DIR / "ground.jpg").string(), 0);
    Texture grassTexture((TEXTURE_DIR / "grass.jpg").string(), 1);
    Texture mountainTexture((TEXTURE_DIR / "mountain.jpg").string(), 2); // Using stoneWall as mountain texture

    // Generate terrain
    std::cout << "[terraintest] Generating terrain..." << std::endl;
    TerrainConfig config{
        .width = 256,
        .height = 256,
        .scale = 25.0f,         // Scale for terrain features
        .heightScale = 100.0f,  // 2x taller (was 80) for massive mountain
        .octaves = 3,           // Keep octaves low for sharp features
        .persistence = 0.9f,    // More variation
        .lacunarity = 1.5f,     // Slightly higher for more detail
        .vertexStep = 4         // Generate vertices every 2 units (4x fewer vertices, same world size)
    };
    
    TerrainGenerator terrainGen(config);
    Mesh* terrainMesh = terrainGen.generateTerrain();
    std::cout << "[terraintest] Terrain generated!" << std::endl;

    // Create terrain renderable
    MeshRenderable* terrain = new MeshRenderable(terrainMesh, &terrainShader);
    terrain->setTransform(glm::mat4(1.0f));
    scene.addRenderable(terrain);

    // Bind textures to shader
    terrainShader.bind();
    groundTexture.bind();
    terrainShader.setUniform("u_texture0", 0);
    grassTexture.bind();
    terrainShader.setUniform("u_texture1", 1);
    mountainTexture.bind();
    terrainShader.setUniform("u_texture2", 2);

    std::cout << "[terraintest] Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  Q/E - Move down/up" << std::endl;
    std::cout << "  Shift - Move faster" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;

    FrameTimer frameTimer;
    int frameCount = 0;
    
    while (!glfwWindowShouldClose(g_window))
    {
        scene.tick();
        float dt = frameTimer.getDeltaTime();
        
        MovementInput movementInput = getUserMovementInput(g_window);
        updateCameraFreeFly(scene.m_activeCamera, movementInput, dt);
        
        // Display FPS every 60 frames
        if (frameCount % 60 == 0)
        {
            std::cout << "\r[terraintest] FPS: " << std::fixed << std::setprecision(1) << (1.0f / dt) << "     " << std::flush;
        }
        frameCount++;
        
        scene.renderScene();

        // Press ESC to exit
        if (glfwGetKey(g_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(g_window, true);
    }

    std::cout << std::endl; // Newline after FPS counter

    // Cleanup
    delete terrain;
    delete terrainMesh->vertexArray;
    delete terrainMesh->indexBuffer;
    delete terrainMesh;
    
    oogaboogaExit();
    return 0;
}
