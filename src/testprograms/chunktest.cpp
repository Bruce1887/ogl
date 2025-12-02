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

int main(int, char **)
{
    if (oogaboogaInit("Chunked Terrain Test"))
        return -1;
    {
        CameraConfiguration camConfig{
            .fov = 45.0f,
            .aspect = (float)window_X / (float)window_Y,
            .near = 0.1f,
            .far = 2000.0f};

        // Create terrain generator (optimized config for best visuals)
        TerrainConfig config{
            .width = 256,
            .height = 256,
            .scale = 25.0f,        // Scale for terrain features
            .heightScale = 100.0f, // Mountain height
            .octaves = 2,          // Increased from 3 for more detail
            .persistence = 0.4f,   // Better balanced (was 0.9f)
            .lacunarity = 3.0f,    // Standard value for natural terrain
            .vertexStep = 1        // Not used by chunks
        };
        TerrainGenerator terrainGen(config);

        // Camera setup - EXACT same as terraintest
        Camera camera(camConfig);
        camera.m_Position = glm::vec3(100.0f, 80.0f, 100.0f); // Same as terraintest
        camera.m_Target = glm::vec3(-50.0f, 60.0f, -50.0f);   // Same as terraintest
        camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
        camera.WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

        PhongLightConfig lightCfg{
            .lightPosition = glm::vec3(200.0f, 300.0f, 200.0f),
            .ambientLight = glm::vec3(0.4f, 0.4f, 0.45f),
            .diffuseLight = glm::vec3(0.9f, 0.9f, 0.8f),
            .specularLight = glm::vec3(0.5f, 0.5f, 0.5f)};

        LightSource lightSource = LightSource::fromConfig(lightCfg);
        Scene scene(camera, std::move(lightSource));

        // Shader for terrain
        std::shared_ptr<Shader> terrainShader = std::make_shared<Shader>();
        terrainShader->addShader("Terrain.vert", ShaderType::VERTEX);
        terrainShader->addShader("TerrainBlend.frag", ShaderType::FRAGMENT);
        terrainShader->createProgram();

        // Load and setup textures
        std::shared_ptr<Texture> groundTexture = std::make_shared<Texture>((TEXTURE_DIR / "ground.jpg").string(), 0);
        groundTexture->targetUniform = "u_texture0";
        std::shared_ptr<Texture> grassTexture = std::make_shared<Texture>((TEXTURE_DIR / "grass.jpg").string(), 1);
        grassTexture->targetUniform = "u_texture1";
        std::shared_ptr<Texture> mountainTexture = std::make_shared<Texture>((TEXTURE_DIR / "mountain.jpg").string(), 2);
        mountainTexture->targetUniform = "u_texture2";
        std::shared_ptr<Texture> blueWaterTexture = std::make_shared<Texture>((TEXTURE_DIR / "blueWater.jpg").string(), 3);
        blueWaterTexture->targetUniform = "u_texture3";
        std::shared_ptr<Texture> whiteWaterTexture = std::make_shared<Texture>((TEXTURE_DIR / "whiteWater.jpg").string(), 4);
        whiteWaterTexture->targetUniform = "u_texture4";
        std::vector<std::shared_ptr<Texture>> terrainTextures = {
            groundTexture,
            grassTexture,
            mountainTexture,
            blueWaterTexture,
            whiteWaterTexture};

        // Optimized chunk settings
        DEBUG_PRINT("Setting up chunked terrain system...");
        int chunkSize = 100; // Clean 100x100 chunks
        //int vertexStep = 5;  // 5 divides 100 evenly -> 20x20 grid per chunk
        int vertexStep = 20;  
        int gc_threshold = 400;
        TerrainChunkManager chunkManager(&terrainGen, chunkSize, vertexStep, terrainTextures, gc_threshold);
        chunkManager.setShader(terrainShader);
        float renderDistance = 40; //  180.0f; // Optimized: slightly reduced from 200 for better performance

        DEBUG_PRINT("Chunked terrain system ready!");
        DEBUG_PRINT("Optimization settings:"
                    << "  - Chunk size: " << chunkSize << "x" << chunkSize
                    << "  - Vertex step: " << vertexStep << " (" << (chunkSize / vertexStep) << "x" << (chunkSize / vertexStep) << " grid per chunk)"
                    << "  - Vertices per chunk: ~" << ((chunkSize / vertexStep) * (chunkSize / vertexStep) * 6)
                    << "  - Render distance: " << renderDistance << " units");

        FrameTimer frameTimer;
        int frameCount = 0;

        while (!glfwWindowShouldClose(g_window))
        {
            scene.tick();
            float dt = frameTimer.getDeltaTime();

            // Update camera with new input system
            scene.m_activeCamera.flyControl(g_InputManager, dt);

            // Update chunks based on UPDATED camera position
            chunkManager.updateChunks(scene.m_activeCamera.m_Position, renderDistance);

            for (const auto &chunkPtr : chunkManager.m_chunks)
            {
                chunkPtr->render(scene.m_activeCamera.getViewMatrix(), scene.m_activeCamera.getProjectionMatrix(), &scene.m_lightSource.config);
            }

            // Display chunk count every 60 frames
            if (frameCount % 120 == 0)
            {
                DEBUG_PRINT("Loaded chunks: " << chunkManager.m_chunks.size()
                                              << " | FPS: " << std::fixed << std::setprecision(1) << (1.0f / dt) << "     ");
            }
            frameCount++;

            scene.renderScene();

            if (glfwGetKey(g_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(g_window, true);
        }

        std::cout << std::endl;
    }
    oogaboogaExit();
    return 0;
}
