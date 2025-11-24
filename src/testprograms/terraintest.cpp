#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "MeshRenderable.h"
#include "TerrainGenerator.h"
#include "Frametimer.h"
#include <iomanip>

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
    if (oogaboogaInit("Terrain Test - Perlin Noise"))
        return -1;

    // Setup mouse input
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Setup input manager
    InputManager inputManager;
    glfwSetWindowUserPointer(g_window, &inputManager);
    glfwSetKeyCallback(g_window, key_callback);
    glfwSetCursorPosCallback(g_window, cursor_position_callback);
    glfwSetScrollCallback(g_window, scroll_callback);
    
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
    camera.WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

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
        
        // Update camera with new input system
        scene.m_activeCamera.flyControl(&inputManager, dt);
        
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
