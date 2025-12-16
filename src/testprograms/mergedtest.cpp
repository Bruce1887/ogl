#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "Terrain/TerrainGenerator.h"
#include "Terrain/TerrainChunk.h"
#include "game/Player.h"
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
        TerrainChunkManager chunkManager(&terrainGen, {groundTexture, grassTexture, mountainTexture, blueWaterTexture, whiteWaterTexture});
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
        Player player(glm::vec3(100, 0, 100), (MODELS_DIR / "BruceMob" / "BruceMob.obj").string());
        player.modelScale = 0.3f;  // Make the player model smaller
        player.modelYOffset = 0.0f;

        // Set fog uniforms for the player model
        player.playerModel.setFogUniforms(fogColor, fogStart, fogEnd);

        // Spawn enemies at random positions near player
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
        std::uniform_real_distribution<float> distanceDist(3.0f, 6.0f); // Enemies spawn very close (3-6 units away)

        // Helper lambda to spawn at random position around player
        auto getRandomSpawnPos = [&]() {
            float angle = glm::radians(angleDist(gen));
            float dist = distanceDist(gen);
            return player.position + glm::vec3(cos(angle) * dist, 0.0f, sin(angle) * dist);
        };

        // Enemy 2: BobboMob - BLIND, follows smell (long range detection)
        Enemy bobbomob(player.position + glm::vec3(-3.0f, 0.0f, 0.0f), (MODELS_DIR / "BobboMob" / "BobboMob.obj").string());
        bobbomob.modelYOffset = 0.0f;
        bobbomob.modelScale = 0.35f;  
        bobbomob.detectionRange = 200.0f;  // Can smell player from far away
        bobbomob.moveSpeed = 8.0f;         // Normal speed
        bobbomob.movementPattern = MovementPattern::DIRECT;  // Follows smell directly
        bobbomob.enemyModel.setFogUniforms(fogColor, fogStart, fogEnd);

        // Enemy 3: jompamob - Has eyes, shorter detection but cautious
        Enemy jompamob(player.position + glm::vec3(3.0f, 0.0f, 0.0f), (MODELS_DIR / "JompaMob" / "JompaMob.obj").string());
        jompamob.modelYOffset = 0.0f;
        jompamob.modelScale = 0.32f;  
        jompamob.detectionRange = 75.0f;   // Can only see at 75 units
        jompamob.moveSpeed = 6.0f;         // Slightly slower than standard
        jompamob.movementPattern = MovementPattern::CAUTIOUS;  // Moves carefully
        jompamob.enemyModel.setFogUniforms(fogColor, fogStart, fogEnd);

        // Enemy 4: MangeMob - Big, fast but DUMB (zigzag pattern)
        Enemy mangeMob(player.position + glm::vec3(0.0f, 0.0f, -3.0f), (MODELS_DIR / "MangeMob" / "MangeMob.obj").string());
        mangeMob.modelYOffset = 0.0f;
        mangeMob.modelScale = 0.45f;  
        mangeMob.detectionRange = 100.0f;  // Normal detection
        mangeMob.moveSpeed = 12.0f;        // Fast!
        mangeMob.movementPattern = MovementPattern::ZIGZAG;  // Dumb zigzag movement
        mangeMob.health = 200.0f;  // Extra health
        mangeMob.maxHealth = 200.0f;  // Extra health
        mangeMob.enemyModel.setFogUniforms(fogColor, fogStart, fogEnd);
        

        // Create enemy list for attack system
        std::vector<Enemy*> enemies = { &bobbomob, &jompamob, &mangeMob };

        while (!glfwWindowShouldClose(g_window))
        {
            scene.tick();
            float dt = frameTimer.getDeltaTime();

            player.update(dt, g_InputManager, &chunkManager);
            
            // Check for attack input (left click or K)
            if (g_InputManager->attackInput.fetchAttack())
            {
                int hits = player.attack(enemies);
                if (hits > 0)
                    DEBUG_PRINT("Hit " << hits << " enemies!");
            }
            
            // Respawn dead enemies near the player
            for (Enemy* enemy : enemies)
            {
                if (enemy->isDead())
                {
                    enemy->respawn(player.position, 15.0f, 40.0f);
                    DEBUG_PRINT("Enemy respawned!");
                }
            }
            
            // Update enemies
            bobbomob.update(dt, &player, &chunkManager);
            jompamob.update(dt, &player, &chunkManager);
            mangeMob.update(dt, &player, &chunkManager);

            camController.handlePanning(dt); // uses GLFW directly
            camController.update(scene.m_activeCamera, player);

            player.render(scene.m_activeCamera.getViewMatrix(),
                          scene.m_activeCamera.getProjectionMatrix(),
                          &scene.m_lightSource.config);

            // Render enemies
            bobbomob.render(scene.m_activeCamera.getViewMatrix(),
                            scene.m_activeCamera.getProjectionMatrix(),
                            &scene.m_lightSource.config);

            jompamob.render(scene.m_activeCamera.getViewMatrix(),
                            scene.m_activeCamera.getProjectionMatrix(),
                            &scene.m_lightSource.config);

            mangeMob.render(scene.m_activeCamera.getViewMatrix(),
                            scene.m_activeCamera.getProjectionMatrix(),
                            &scene.m_lightSource.config);

            chunkManager.updateChunks(scene.m_activeCamera.m_Position);

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