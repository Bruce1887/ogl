#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "TerrainGenerator.h"
#include "TerrainChunk.h"
#include "Frametimer.h"
#include <sstream>
#include <iomanip>

// Free-fly camera controller
static void updateCameraFreeFly(Camera &camera, MovementInput movementInput, float deltaTime)
{
    float baseSpeed = 30.0f;
    if (movementInput.shiftDown)
        baseSpeed *= 3.0f;
    float speed = baseSpeed * deltaTime;

    glm::vec3 forwardDir = glm::normalize(camera.m_Target - camera.m_Position);
    glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, camera.m_Up));

    camera.m_Position += forwardDir * (float)movementInput.wasd.forward * speed;
    camera.m_Position += rightDir * (float)movementInput.wasd.right * speed;

    if (movementInput.qDown)
        camera.m_Position += glm::vec3(0.0f, -1.0f, 0.0f) * speed;
    if (movementInput.eDown)
        camera.m_Position += glm::vec3(0.0f, 1.0f, 0.0f) * speed;

    float targetDistance = 50.0f;
    camera.m_Target = camera.m_Position + forwardDir * targetDistance;
}

static void handleMouseLook(Camera &camera, double xoffset, double yoffset)
{
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    glm::vec3 direction = glm::normalize(camera.m_Target - camera.m_Position);
    float radius = glm::length(camera.m_Target - camera.m_Position);
    float yaw = glm::degrees(atan2(direction.z, direction.x));
    float pitch = glm::degrees(asin(direction.y));

    yaw += xoffset;
    pitch -= yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 newDirection;
    newDirection.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    newDirection.y = sin(glm::radians(pitch));
    newDirection.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

    camera.m_Target = camera.m_Position + glm::normalize(newDirection) * radius;
}

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
    if (oogaboogaInit("Chunked Terrain Test"))
        return -1;

    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    CameraConfiguration camConfig{ 
        .fov = 60.0f, 
        .aspect = (float)window_X / (float)window_Y, 
        .near = 0.1f, 
        .far = 2000.0f 
    };
    
    // Create terrain generator (optimized config for best visuals)
    TerrainConfig config{
        .width = 256,
        .height = 256,
        .scale = 25.0f,         // Scale for terrain features
        .heightScale = 100.0f,  // Mountain height
        .octaves = 4,           // Increased from 3 for more detail
        .persistence = 0.5f,    // Better balanced (was 0.9f)
        .lacunarity = 2.0f,     // Standard value for natural terrain
        .vertexStep = 1         // Not used by chunks
    };
    TerrainGenerator terrainGen(config);
    
    // Camera setup - EXACT same as terraintest
    Camera camera(camConfig);
    camera.m_Position = glm::vec3(100.0f, 80.0f, 100.0f);  // Same as terraintest
    camera.m_Target = glm::vec3(-50.0f, 60.0f, -50.0f);     // Same as terraintest
    camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

    glfwSetWindowUserPointer(g_window, &camera);
    glfwSetCursorPosCallback(g_window, mouse_callback);

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

    // Load textures (same as terraintest)
    Texture groundTexture((TEXTURE_DIR / "ground.jpg").string(), 0);
    Texture grassTexture((TEXTURE_DIR / "grass.jpg").string(), 1);
    Texture mountainTexture((TEXTURE_DIR / "mountain.jpg").string(), 2);

    terrainShader.bind();
    groundTexture.bind();
    terrainShader.setUniform("u_texture0", 0);
    grassTexture.bind();
    terrainShader.setUniform("u_texture1", 1);
    mountainTexture.bind();
    terrainShader.setUniform("u_texture2", 2);

    // Optimized chunk settings
    std::cout << "[chunktest] Setting up chunked terrain system..." << std::endl;
    int chunkSize = 100;
    int vertexStep = 3;  // Optimized: 3 gives good balance (9x fewer vertices than step 1)
    TerrainChunkManager chunkManager(&terrainGen, chunkSize, vertexStep);
    chunkManager.setShader(&terrainShader);
    
    float renderDistance = 180.0f; // Optimized: slightly reduced from 200 for better performance

    std::cout << "[chunktest] Chunked terrain system ready!" << std::endl;
    std::cout << "[chunktest] Optimization settings:" << std::endl;
    std::cout << "  - Chunk size: " << chunkSize << "x" << chunkSize << std::endl;
    std::cout << "  - Vertex step: " << vertexStep << " (" << (chunkSize/vertexStep) << "x" << (chunkSize/vertexStep) << " grid per chunk)" << std::endl;
    std::cout << "  - Vertices per chunk: ~" << ((chunkSize/vertexStep) * (chunkSize/vertexStep) * 6) << std::endl;
    std::cout << "  - Render distance: " << renderDistance << " units" << std::endl;
    std::cout << "[chunktest] Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  Q/E - Move down/up" << std::endl;
    std::cout << "  Shift - Move faster" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "  Chunks will load/unload dynamically as you move!" << std::endl;

    FrameTimer frameTimer;
    int frameCount = 0;
    
    while (!glfwWindowShouldClose(g_window))
    {
        scene.tick();
        float dt = frameTimer.getDeltaTime();
        
        MovementInput movementInput = getUserMovementInput(g_window);
        updateCameraFreeFly(scene.m_activeCamera, movementInput, dt);
        
        // Update chunks based on UPDATED camera position
        chunkManager.updateChunks(scene.m_activeCamera.m_Position, renderDistance);
        
        // Clear existing renderables (we'll add chunks each frame)
        scene.clearRenderables();
        
        // Add all loaded chunks to the scene
        for (const auto& pair : chunkManager.getLoadedChunks())
        {
            scene.addRenderable(pair.second);
        }
        
        // Display chunk count every 60 frames
        if (frameCount % 120 == 0)
        {
            std::cout << "\r[chunktest] Loaded chunks: " << chunkManager.getLoadedChunks().size() 
                      << " | FPS: " << std::fixed << std::setprecision(1) << (1.0f / dt) << "     " << std::flush;
        }
        frameCount++;
        
        scene.renderScene();

        if (glfwGetKey(g_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(g_window, true);
    }

    std::cout << std::endl;
    
    oogaboogaExit();
    return 0;
}
