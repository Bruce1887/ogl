#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "Terrain/TerrainGenerator.h"
#include "Terrain/TerrainChunk.h"
#include "Player.h"
#include "game/Enemy.h"
#include "ThirdPersonCamera.h"
#include "Frametimer.h"
#include "Skybox.h"
#include <sstream>
#include <iomanip>
#include <random>

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

        // TerrainGenerator terrainGen(config);
        TerrainGenerator terrainGen;

        // Camera setup
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

        // Setup skybox
        std::vector<std::filesystem::path> skyboxFaces = {
            TEXTURE_DIR / "skybox" / "right.jpg",
            TEXTURE_DIR / "skybox" / "left.jpg",
            TEXTURE_DIR / "skybox" / "top.jpg",
            TEXTURE_DIR / "skybox" / "bottom.jpg",
            TEXTURE_DIR / "skybox" / "front.jpg",
            TEXTURE_DIR / "skybox" / "back.jpg"};
        std::unique_ptr<Skybox> gameSkybox = std::make_unique<Skybox>(skyboxFaces);
        scene.m_skybox = std::move(gameSkybox);

        // Shader for terrain
        std::shared_ptr<Shader> terrainShader = std::make_shared<Shader>();
        terrainShader->addShader("Terrain.vert", ShaderType::VERTEX);
        terrainShader->addShader("TerrainBlend.frag", ShaderType::FRAGMENT);
        terrainShader->createProgram();

        // Load and setup textures
        std::shared_ptr<Texture> groundTexture = Texture::CreateTexture2D((TEXTURE_DIR / "ground.jpg"), "u_texture0");
        std::shared_ptr<Texture> grassTexture = Texture::CreateTexture2D((TEXTURE_DIR / "grass.jpg"), "u_texture1");
        std::shared_ptr<Texture> mountainTexture = Texture::CreateTexture2D((TEXTURE_DIR / "mountain.jpg"), "u_texture2");
        std::shared_ptr<Texture> blueWaterTexture = Texture::CreateTexture2D((TEXTURE_DIR / "blueWater.jpg"), "u_texture3");
        std::shared_ptr<Texture> whiteWaterTexture = Texture::CreateTexture2D((TEXTURE_DIR / "whiteWater.jpg"), "u_texture4");

        // Optimized chunk settings
        int chunkSize = 100; // Clean 100x100 chunks
        int vertexStep = 5;  // 5 divides 100 evenly -> 20x20 grid per chunk        
        int gc_threshold = 35;
        TerrainChunkManager chunkManager(&terrainGen, chunkSize, vertexStep, {groundTexture, grassTexture, mountainTexture, blueWaterTexture, whiteWaterTexture}, gc_threshold);
        chunkManager.setShader(terrainShader);
        float renderDistance = 100.0f;

        // Fog settings (0.51f, 0.90f, 0.95f)
        glm::vec3 fogColor = glm::vec3(0.51f, 0.90f, 0.95f); // Light turquoise fog same color as skybox
        float fogStart = renderDistance * 0.90f;             // Fog starts at 90% of render distance
        float fogEnd = renderDistance * 0.98f;               // Fully opaque at 98% of render distance

        // Set fog uniforms
        terrainShader->bind();
        terrainShader->setUniform("u_fogColor", fogColor);
        terrainShader->setUniform("u_fogStart", fogStart);
        terrainShader->setUniform("u_fogEnd", fogEnd);

        // Set fog uniforms on chunk manager so trees get fog too
        chunkManager.setFogUniforms(fogColor, fogStart, fogEnd);

        FrameTimer frameTimer;
        int frameCount = 0;

        ThirdPersonCamera camController;
        Player player(glm::vec3(100, 0, 100), (MODELS_DIR / "cow" / "cow.obj").string());

        // Set fog uniforms for the player model
        player.playerModel.setFogUniforms(fogColor, fogStart, fogEnd);

        // Spawn enemy at random position near player (sharing model data for performance)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
        std::uniform_real_distribution<float> distanceDist(20.0f, 40.0f); // Enemies spawn 20-40 units away from player
        
        float spawnAngle = glm::radians(angleDist(gen));
        float spawnDistance = distanceDist(gen);
        
        glm::vec3 enemySpawnPos = player.position + glm::vec3(
            cos(spawnAngle) * spawnDistance,   // X offset
            0.0f,                              // Y offset (ground level)
            sin(spawnAngle) * spawnDistance    // Z offset
        );
        
        // Use shared model data from player for better performance
        // Enemy enemy(enemySpawnPos, &player.playerModel);
        Enemy enemy(enemySpawnPos, (MODELS_DIR / "warrior" / "Cyber_Ninja_Warrior.obj").string());
        enemy.modelYOffset = 1.0f;   // Raise model so feet touch ground
        enemy.modelScale = 1.0f;    // Scale down - model is very large
        enemy.enemyModel.setFogUniforms(fogColor, fogStart, fogEnd);

        while (!glfwWindowShouldClose(g_window))
        {
            scene.tick();
            float dt = frameTimer.getDeltaTime();

            player.update(dt, g_InputManager, &chunkManager);
            enemy.update(dt, &player, &chunkManager);

            camController.handlePanning(dt); // uses GLFW directly
            camController.update(scene.m_activeCamera, player);

            player.render(scene.m_activeCamera.getViewMatrix(),
                          scene.m_activeCamera.getProjectionMatrix(),
                          &scene.m_lightSource.config);
            
            enemy.render(scene.m_activeCamera.getViewMatrix(),
                        scene.m_activeCamera.getProjectionMatrix(),
                        &scene.m_lightSource.config);

            chunkManager.updateChunks(scene.m_activeCamera.m_Position, renderDistance);

            for (const auto &chunkPtr : chunkManager.m_chunks)
            {
                chunkPtr->render(scene.m_activeCamera.getViewMatrix(), scene.m_activeCamera.getProjectionMatrix(), &scene.m_lightSource.config);
            }
            
            // Render global water plane (single mesh, no seams between chunks)
            chunkManager.renderWater(scene.m_activeCamera.getViewMatrix(),
                                     scene.m_activeCamera.getProjectionMatrix(),
                                     &scene.m_lightSource.config,
                                     scene.m_activeCamera.m_Position,
                                     renderDistance);
            
            // Render all trees with instanced rendering (single draw call!)
            chunkManager.renderTrees(scene.m_activeCamera.getViewMatrix(), 
                                     scene.m_activeCamera.getProjectionMatrix(), 
                                     &scene.m_lightSource.config);

            // Display chunk count every 120 frames
            if (frameCount % 120 == 0)
            {
                DEBUG_PRINT("Loaded chunks: " << chunkManager.m_chunks.size()
                << " | FPS: " << std::fixed << std::setprecision(1) << (1.0f / dt));
            }
            frameCount++;

            scene.renderScene();

            if (glfwGetKey(g_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(g_window, true);            
        }

    }
    oogaboogaExit();
    return 0;
}