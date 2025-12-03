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
#include "Skybox.h"

// Input callbacks
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    InputManager* inputManager = (InputManager*)glfwGetWindowUserPointer(window);
    if (inputManager)
        inputManager->movementInput.updateMovement(key, action, mods);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    InputManager* inputManager = (InputManager*)glfwGetWindowUserPointer(window);
    if (inputManager)
        inputManager->mouseMoveInput.updateDeltas(xpos, ypos);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    InputManager* inputManager = (InputManager*)glfwGetWindowUserPointer(window);
    if (inputManager)
        inputManager->scrollInput.updateScroll(xoffset, yoffset);
}

int main(int, char **)
{
    if (oogaboogaInit("Chunked Terrain Test"))
        return -1;

    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Setup input manager
    InputManager inputManager;
    glfwSetWindowUserPointer(g_window, &inputManager);
    glfwSetKeyCallback(g_window, key_callback);
    glfwSetCursorPosCallback(g_window, cursor_position_callback);
    glfwSetScrollCallback(g_window, scroll_callback);
    
    CameraConfiguration camConfig{ 
        .fov = 45.0f, 
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
        .persistence = 0.4f,    // Better balanced (was 0.9f)
        .lacunarity = 3.0f,     // Standard value for natural terrain
        .vertexStep = 1         // Not used by chunks
    };
    TerrainGenerator terrainGen(config);
    
    // Camera setup - EXACT same as terraintest
    Camera camera(camConfig);
    camera.m_Position = glm::vec3(100.0f, 80.0f, 100.0f);  // Same as terraintest
    camera.m_Target = glm::vec3(-50.0f, 60.0f, -50.0f);     // Same as terraintest
    camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    PhongLightConfig lightCfg{
        .lightPosition = glm::vec3(200.0f, 300.0f, 200.0f),
        .ambientLight = glm::vec3(0.4f, 0.4f, 0.45f),
        .diffuseLight = glm::vec3(0.9f, 0.9f, 0.8f),
        .specularLight = glm::vec3(0.5f, 0.5f, 0.5f)
    };
    LightSource lightSource{ .config = lightCfg, .visualRepresentation = nullptr };
    Scene scene(camera, lightSource);

    std::vector<std::string> skyboxFaces = {
        "resources/textures/skybox/right.jpg",  // +X
        "resources/textures/skybox/left.jpg",   // -X
        "resources/textures/skybox/top.jpg",    // +Y
        "resources/textures/skybox/bottom.jpg", // -Y
        "resources/textures/skybox/front.jpg",  // +Z
        "resources/textures/skybox/back.jpg"    // -Z
    };

    // 2. Create the Skybox Shader
    Shader* skyboxShader = new Shader();
    // NOTE: Assume your shader loader paths are fixed to work now.
    skyboxShader->addShader("Skybox.vert", ShaderType::VERTEX);
    skyboxShader->addShader("Skybox.frag", ShaderType::FRAGMENT);
    skyboxShader->createProgram();

    // 3. Create the Skybox Object (loads textures and sets up VAO/VBO)
    Skybox* gameSkybox = new Skybox(skyboxFaces);

    // 4. Link the Skybox and Shader to the Scene
    scene.m_skybox = gameSkybox;
    scene.m_skyboxShader = skyboxShader;
    skyboxShader->bind();
    skyboxShader->setUniform("skybox", 0); // Assuming texture slot 0 is the cubemap slot


    // Shader for terrain
    Shader terrainShader;
    terrainShader.addShader("Terrain.vert", ShaderType::VERTEX);
    terrainShader.addShader("TerrainBlend.frag", ShaderType::FRAGMENT);
    terrainShader.createProgram();

    // Load textures (same as terraintest)
    Texture groundTexture((TEXTURE_DIR / "ground.jpg").string(), 0);
    Texture grassTexture((TEXTURE_DIR / "grass.jpg").string(), 1);
    Texture mountainTexture((TEXTURE_DIR / "mountain.jpg").string(), 2);
    Texture blueWaterTexture((TEXTURE_DIR / "blueWater.jpg").string(), 3);
    Texture whiteWaterTexture((TEXTURE_DIR / "whiteWater.jpg").string(), 4);

    terrainShader.bind();
    groundTexture.bind();
    terrainShader.setUniform("u_texture0", 0);
    grassTexture.bind();
    terrainShader.setUniform("u_texture1", 1);
    mountainTexture.bind();
    terrainShader.setUniform("u_texture2", 2);
    blueWaterTexture.bind();
    terrainShader.setUniform("u_texture3", 3);
    whiteWaterTexture.bind();
    terrainShader.setUniform("u_texture4", 4);

    // Optimized chunk settings
    std::cout << "[chunktest] Setting up chunked terrain system..." << std::endl;
    int chunkSize = 100;  // Clean 100x100 chunks
    int vertexStep = 5;   // 5 divides 100 evenly -> 20x20 grid per chunk
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
    
    // Enable blending for water transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    while (!glfwWindowShouldClose(g_window))
    {
        scene.tick();
        float dt = frameTimer.getDeltaTime();
        
        // Update camera with new input system
        scene.m_activeCamera.flyControl(&inputManager, dt);
        
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
