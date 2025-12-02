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
        std::cout << "[chunktest] Setting up chunked terrain system..." << std::endl;
        int chunkSize = 100; // Clean 100x100 chunks
        int vertexStep = 5;  // 5 divides 100 evenly -> 20x20 grid per chunk
        TerrainChunkManager chunkManager(&terrainGen, chunkSize, vertexStep, terrainTextures);
        chunkManager.setShader(terrainShader);
        float renderDistance = 250.0f; // Extended render distance for more distant fog
        
        // Fog settings - light fog starting early, becoming dense at chunk limit
        float fogStart = 150.0f;  // Subtle fog begins at 80 units
        float fogEnd = renderDistance * 0.95f;  // Dense fog at 90% of render distance to hide chunk loading
        glm::vec3 fogColor = glm::vec3(0.7f, 0.8f, 0.9f);  // Light blue-grey fog color (sky-like)
        
        // Set background color to fog color so the sky matches the fog
        glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0f);

        std::cout << "[chunktest] Chunked terrain system ready!" << std::endl;
        std::cout << "[chunktest] Optimization settings:" << std::endl;
        std::cout << "  - Chunk size: " << chunkSize << "x" << chunkSize << std::endl;
        std::cout << "  - Vertex step: " << vertexStep << " (" << (chunkSize / vertexStep) << "x" << (chunkSize / vertexStep) << " grid per chunk)" << std::endl;
        std::cout << "  - Vertices per chunk: ~" << ((chunkSize / vertexStep) * (chunkSize / vertexStep) * 6) << std::endl;
        std::cout << "  - Render distance: " << renderDistance << " units" << std::endl;
        std::cout << "  - Fog start: " << fogStart << " units" << std::endl;
        std::cout << "  - Fog end: " << fogEnd << " units" << std::endl;
        std::cout << "[chunktest] Controls:" << std::endl;
        std::cout << "  WASD - Move camera" << std::endl;
        std::cout << "  Mouse - Look around" << std::endl;
        std::cout << "  Shift - Move faster" << std::endl;
        std::cout << "  ESC - Exit" << std::endl;
        std::cout << "  Chunks will load/unload dynamically as you move!" << std::endl;

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
                // Set fog uniforms for each chunk's terrain
                chunkPtr->terrain_mr->setUniform("u_fogStart", fogStart);
                chunkPtr->terrain_mr->setUniform("u_fogEnd", fogEnd);
                chunkPtr->terrain_mr->setUniform("u_fogColor", fogColor);
                
                chunkPtr->render(scene.m_activeCamera.getViewMatrix(), scene.m_activeCamera.getProjectionMatrix(), &scene.m_lightSource.config);
            }

            // Display chunk count every 60 frames
            if (frameCount % 120 == 0)
            {
                std::cout << "\r[chunktest] Loaded chunks: " << chunkManager.m_chunks.size()
                          << " | FPS: " << std::fixed << std::setprecision(1) << (1.0f / dt) << "     " << std::flush;
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
