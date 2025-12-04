#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "Terrain/TerrainGenerator.h"
#include "Terrain/TerrainChunk.h"
#include "Player.h"
#include "ThirdPersonCamera.h"
#include "Frametimer.h"
#include "Skybox.h"
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
        std::vector<std::string> skyboxFaces = {
            "resources/textures/skybox/right.jpg",  
            "resources/textures/skybox/left.jpg",   
            "resources/textures/skybox/top.jpg",    
            "resources/textures/skybox/bottom.jpg", 
            "resources/textures/skybox/front.jpg",  
            "resources/textures/skybox/back.jpg"    
        };

        // Create the Skybox Shader
        Shader* skyboxShader = new Shader();
        skyboxShader->addShader("Skybox.vert", ShaderType::VERTEX);
        skyboxShader->addShader("Skybox.frag", ShaderType::FRAGMENT);
        skyboxShader->createProgram();

        // Create the Skybox Object (loads textures and sets up VAO/VBO)
        Skybox* gameSkybox = new Skybox(skyboxFaces);

        // Link the Skybox and Shader to the Scene
        scene.m_skybox = gameSkybox;
        scene.m_skyboxShader = skyboxShader;
        skyboxShader->bind();
        skyboxShader->setUniform("skybox", 0); // Texture slot 0 for cubemap

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
        int vertexStep = 5;  
        int gc_threshold = 50;
        TerrainChunkManager chunkManager(&terrainGen, chunkSize, vertexStep, terrainTextures, gc_threshold);
        chunkManager.setShader(terrainShader);
        float renderDistance = 100.0f;
        
        // Fog settings (0.51f, 0.90f, 0.95f)
        glm::vec3 fogColor = glm::vec3(0.51f, 0.90f, 0.95f); // Light turquoise fog same color as skybox
        float fogStart = renderDistance * 0.90f; // Fog starts at 90% of render distance
        float fogEnd = renderDistance * 0.98f;   // Fully opaque at 98% of render distance
        
        // Set fog uniforms
        terrainShader->bind();
        terrainShader->setUniform("u_fogColor", fogColor);
        terrainShader->setUniform("u_fogStart", fogStart);
        terrainShader->setUniform("u_fogEnd", fogEnd);
        
        // Set fog uniforms on chunk manager so trees get fog too
        chunkManager.setFogUniforms(fogColor, fogStart, fogEnd);

        DEBUG_PRINT("Chunked terrain system ready!");
        DEBUG_PRINT("Optimization settings:"
                    << "  - Chunk size: " << chunkSize << "x" << chunkSize
                    << "  - Vertex step: " << vertexStep << " (" << (chunkSize / vertexStep) << "x" << (chunkSize / vertexStep) << " grid per chunk)"
                    << "  - Vertices per chunk: ~" << ((chunkSize / vertexStep) * (chunkSize / vertexStep) * 6)
                    << "  - Render distance: " << renderDistance << " units");

        FrameTimer frameTimer;
        int frameCount = 0;


        ThirdPersonCamera camController;    
        Player player(glm::vec3(100, 0, 100), (MODELS_DIR /  "cow" / "cow.obj").string());
        
        // Set fog uniforms for the player model
        player.playerModel.setFogUniforms(fogColor, fogStart, fogEnd);

        while (!glfwWindowShouldClose(g_window))
        {
            scene.tick();
            float dt = frameTimer.getDeltaTime();


            player.update(dt, g_InputManager, &chunkManager);

            camController.handlePanning(dt); // uses GLFW directly
            camController.update(scene.m_activeCamera, player);

            player.render(scene.m_activeCamera.getViewMatrix(),
              scene.m_activeCamera.getProjectionMatrix(),
              &scene.m_lightSource.config);

            
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

        // Cleanup skybox resources
        delete gameSkybox;
        delete skyboxShader;
    }
    oogaboogaExit();
    return 0;
}