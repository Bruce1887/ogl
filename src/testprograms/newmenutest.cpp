#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <iomanip>
#include <random>

#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "Terrain/TerrainGenerator.h"
#include "Terrain/TerrainChunk.h"
#include "ThirdPersonCamera.h"
#include "Frametimer.h"
#include "Skybox.h"
#include "RenderingContext.h"

#include "game/Player.h"
#include "game/GameClock.h"
#include "game/Enemy.h"
#include "ui/UIManager.h"
#include "ui/GameState.h"

// Forward declarations
void initializeWorld(Scene **scene, TerrainGenerator **terrainGen, TerrainChunkManager **chunkManager,
                     Player **player, Enemy **enemy, GameClock **gameClock,
                     ThirdPersonCamera **camController, float *renderDistance);
void cleanupWorld(Scene *scene, TerrainGenerator *terrainGen, TerrainChunkManager *chunkManager,
                  Player *player, Enemy *enemy, GameClock *gameClock,
                  ThirdPersonCamera *camController);

// Module-level mouse tracking for GLFW callbacks
namespace
{
    double g_cursorX = 0.0;
    double g_cursorY = 0.0;
}

void cursorPosCallback(GLFWwindow * /*window*/, double xpos, double ypos)
{
    g_cursorX = xpos;
    g_cursorY = ypos;
}

int main(int, char **)
{
    if (oogaboogaInit(__FILE__))
        return -1;

    {
        // Create UI Manager
        ui::UIManager uiManager(window_X, window_Y);

        // World objects (created when loading)
        Scene *scene = nullptr;
        TerrainGenerator *terrainGen = nullptr;
        TerrainChunkManager *chunkManager = nullptr;
        Player *player = nullptr;
        Enemy *enemy = nullptr;
        GameClock *gameClock = nullptr;
        ThirdPersonCamera *camController = nullptr;
        float renderDistance = 100.0f;

        // Setup menu skybox (filesystem paths; Skybox manages its own shader)
        std::vector<std::filesystem::path> menuSkyboxFaces = {
            TEXTURE_DIR / "skybox" / "right.jpg",
            TEXTURE_DIR / "skybox" / "left.jpg",
            TEXTURE_DIR / "skybox" / "top.jpg",
            TEXTURE_DIR / "skybox" / "bottom.jpg",
            TEXTURE_DIR / "skybox" / "front.jpg",
            TEXTURE_DIR / "skybox" / "back.jpg"};
        auto menuSkybox = std::make_unique<Skybox>(menuSkyboxFaces);
        uiManager.setMenuSkybox(std::move(menuSkybox), nullptr);

        // Set up UIManager callbacks
        uiManager.onStartGame = [&]()
        {
            std::cout << "Initializing world..." << std::endl;
            initializeWorld(&scene, &terrainGen, &chunkManager, &player, &enemy, &gameClock, &camController, &renderDistance);
            uiManager.initializeGameUI(player, gameClock);
        };

        uiManager.onResumeGame = [&]()
        {
            // Cursor mode handled automatically by UIManager
        };

        uiManager.onQuitGame = [&]()
        {
            glfwSetWindowShouldClose(g_window, GLFW_TRUE);
        };

        // Set up cursor callback (using static lambda like the old code)
        glfwSetCursorPosCallback(g_window, cursorPosCallback);

        FrameTimer frameTimer;
        int frameCount = 0;

        // ensure cursor mode is correct at startup (old code set it explicitly)
        glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        // Initialize lastShouldShowCursor so the first loop iteration will set the mode if needed.
        bool lastShouldShowCursor = !uiManager.shouldShowCursor();
        bool mouseButtonWasPressed = false;

        while (!glfwWindowShouldClose(g_window))
        {
            float dt = frameTimer.getDeltaTime();

            glfwPollEvents();

            // Update cursor mode based on UI state (do this after poll so any key events changed state)
            bool shouldShowCursor = uiManager.shouldShowCursor();
            if (shouldShowCursor != lastShouldShowCursor)
            {
                glfwSetInputMode(g_window, GLFW_CURSOR, shouldShowCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
                lastShouldShowCursor = shouldShowCursor;
            }

            // Handle ESC key for pause
            static bool escPressedLastFrame = false;
            bool escPressed = glfwGetKey(g_window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
            if (escPressed && !escPressedLastFrame)
            {
                uiManager.handleKeyPress(GLFW_KEY_ESCAPE, GLFW_PRESS);
            }
            escPressedLastFrame = escPressed;

            // Handle mouse input for UI if cursor is visible
            if (shouldShowCursor)
            {
                // cursor pos is updated by callback into g_cursorX/g_cursorY
                uiManager.handleMouseMove(g_cursorX, g_cursorY);
                bool mousePressed = glfwGetMouseButton(g_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
                // Many UIs expect clicks to be initiated on press; forwarding release can confuse things.
                if (mousePressed && !mouseButtonWasPressed)
                {
                    uiManager.handleMouseClick(g_cursorX, g_cursorY, true); // pressed
                }
                // Optionally forward releases if your UI needs them:
                else if (!mousePressed && mouseButtonWasPressed)
                {
                    uiManager.handleMouseClick(g_cursorX, g_cursorY, false); // released
                }

                mouseButtonWasPressed = mousePressed;
            }

            // Update UI Manager
            uiManager.update(dt);

            // Update game world if playing and not paused
            if (uiManager.getCurrentState() == ui::GameState::PLAYING && !uiManager.isPaused())
            {
                if (gameClock)
                {
                    gameClock->Update(dt);
                }

                if (player && chunkManager)
                {
                    player->update(dt, g_InputManager, chunkManager);
                }

                if (enemy && player && chunkManager)
                {
                    enemy->update(dt, player, chunkManager);
                }

                if (camController && scene && player)
                {
                    camController->handlePanning(dt);
                    camController->update(scene->m_activeCamera, *player);
                }

                if (chunkManager && scene)
                {
                    chunkManager->updateChunks(scene->m_activeCamera.m_Position);
                }
            }

            // Render
            glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render world if playing (even when paused, show world behind pause menu)
            if (uiManager.getCurrentState() == ui::GameState::PLAYING && scene)
            {
                glEnable(GL_DEPTH_TEST);
                glDepthMask(GL_TRUE);
                glEnable(GL_CULL_FACE);
                glDisable(GL_BLEND);

                // Render player
                if (player)
                {
                    player->render(scene->m_activeCamera.getViewMatrix(),
                                   scene->m_activeCamera.getProjectionMatrix(),
                                   &scene->m_lightSource.config);
                }

                // Render enemy
                if (enemy)
                {
                    enemy->render(scene->m_activeCamera.getViewMatrix(),
                                  scene->m_activeCamera.getProjectionMatrix(),
                                  &scene->m_lightSource.config);
                }

                // Render terrain chunks
                if (chunkManager)
                {
                    for (const auto &chunk : chunkManager->m_chunks)
                    {
                        chunk->render(scene->m_activeCamera.getViewMatrix(),
                                      scene->m_activeCamera.getProjectionMatrix(),
                                      &scene->m_lightSource.config);
                    }
                }

                // Global water
                if (chunkManager && scene)
                {
                    chunkManager->renderWater(scene->m_activeCamera.getViewMatrix(),
                                              scene->m_activeCamera.getProjectionMatrix(),
                                              &scene->m_lightSource.config,
                                              scene->m_activeCamera.m_Position,
                                              renderDistance);
                }

                // Trees (instanced)
                if (chunkManager && scene)
                {
                    chunkManager->renderTrees(scene->m_activeCamera.getViewMatrix(),
                                              scene->m_activeCamera.getProjectionMatrix(),
                                              &scene->m_lightSource.config);
                }

                // Render skybox and scene
                scene->renderScene();
            }

            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Render UI on top
            uiManager.render(glm::mat4(1.0f), glm::mat4(1.0f));

            // Stats
            if (chunkManager && frameCount % 120 == 0 &&
                uiManager.getCurrentState() == ui::GameState::PLAYING && !uiManager.isPaused())
            {
                DEBUG_PRINT("Chunks: " << chunkManager->m_chunks.size()
                                       << " | FPS: " << std::fixed << std::setprecision(1)
                                       << (1.0f / dt));
            }

            frameCount++;
            glfwSwapBuffers(g_window);
        }

        // Cleanup
        if (scene)
            cleanupWorld(scene, terrainGen, chunkManager, player, enemy, gameClock, camController);
    }
    DEBUG_PRINT("Exiting program.");
    oogaboogaExit();
    return 0;
}

void initializeWorld(Scene **scene, TerrainGenerator **terrainGen, TerrainChunkManager **chunkManager,
                     Player **player, Enemy **enemy, GameClock **gameClock,
                     ThirdPersonCamera **camController, float *renderDistance)
{
    // Camera setup
    CameraConfiguration camConfig{
        .fov = 45.0f,
        .aspect = (float)window_X / (float)window_Y,
        .near = 0.1f,
        .far = 2000.0f};

    Camera camera(camConfig);
    camera.m_Position = glm::vec3(100.0f, 80.0f, 100.0f);
    camera.m_Target = glm::vec3(-50.0f, 60.0f, -50.0f);
    camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Lighting
    PhongLightConfig lightCfg{
        .lightPosition = glm::vec3(200.0f, 300.0f, 200.0f),
        .ambientLight = glm::vec3(0.4f, 0.4f, 0.45f),
        .diffuseLight = glm::vec3(0.9f, 0.9f, 0.8f),
        .specularLight = glm::vec3(0.5f, 0.5f, 0.5f)};

    LightSource lightSource = LightSource::fromConfig(lightCfg);
    *scene = new Scene(camera, std::move(lightSource));

    // Skybox (Scene owns unique_ptr; Skybox manages its shader)
    std::vector<std::filesystem::path> skyboxFaces = {
        TEXTURE_DIR / "skybox" / "right.jpg",
        TEXTURE_DIR / "skybox" / "left.jpg",
        TEXTURE_DIR / "skybox" / "top.jpg",
        TEXTURE_DIR / "skybox" / "bottom.jpg",
        TEXTURE_DIR / "skybox" / "front.jpg",
        TEXTURE_DIR / "skybox" / "back.jpg"};

    (*scene)->m_skybox = std::make_unique<Skybox>(skyboxFaces);

    // Terrain
    *terrainGen = new TerrainGenerator();

    std::shared_ptr<Shader> terrainShader = std::make_shared<Shader>();
    terrainShader->addShader("Terrain.vert", ShaderType::VERTEX);
    terrainShader->addShader("TerrainBlend.frag", ShaderType::FRAGMENT);
    terrainShader->createProgram();

    // Use CreateTexture2D instead of old constructor
    std::shared_ptr<Texture> groundTexture = Texture::CreateTexture2D(TEXTURE_DIR / "ground.jpg", "u_texture0");
    std::shared_ptr<Texture> grassTexture = Texture::CreateTexture2D(TEXTURE_DIR / "grass.jpg", "u_texture1");
    std::shared_ptr<Texture> mountainTexture = Texture::CreateTexture2D(TEXTURE_DIR / "mountain.jpg", "u_texture2");
    std::shared_ptr<Texture> blueWaterTexture = Texture::CreateTexture2D(TEXTURE_DIR / "blueWater.jpg", "u_texture3");
    std::shared_ptr<Texture> whiteWaterTexture = Texture::CreateTexture2D(TEXTURE_DIR / "whiteWater.jpg", "u_texture4");

    std::vector<std::shared_ptr<Texture>> terrainTextures = {
        groundTexture, grassTexture, mountainTexture, blueWaterTexture, whiteWaterTexture};

    *chunkManager = new TerrainChunkManager(*terrainGen, terrainTextures);
    (*chunkManager)->setShader(terrainShader);

    // Fog settings aligned with mergedtest
    *renderDistance = 100.0f;
    glm::vec3 fogColor = glm::vec3(0.51f, 0.90f, 0.95f);
    float fogStart = (*renderDistance) * 0.90f;
    float fogEnd = (*renderDistance) * 0.98f;

    terrainShader->bind();
    terrainShader->setUniform("u_fogColor", fogColor);
    terrainShader->setUniform("u_fogStart", fogStart);
    terrainShader->setUniform("u_fogEnd", fogEnd);
    (*chunkManager)->setFogUniforms(fogColor, fogStart, fogEnd);

    // Player
    *player = new Player(glm::vec3(100, 0, 100), (MODELS_DIR / "cow" / "cow.obj").string());
    (*player)->playerModel.setFogUniforms(fogColor, fogStart, fogEnd);

    // Enemy spawn near player
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
        std::uniform_real_distribution<float> distanceDist(20.0f, 40.0f);
        float spawnAngle = glm::radians(angleDist(gen));
        float spawnDistance = distanceDist(gen);
        glm::vec3 enemySpawnPos = (*player)->position + glm::vec3(
                                                            cos(spawnAngle) * spawnDistance,
                                                            0.0f,
                                                            sin(spawnAngle) * spawnDistance);
        *enemy = new Enemy(enemySpawnPos, (MODELS_DIR / "cow" / "cow.obj").string());
        (*enemy)->modelYOffset = 1.0f;
        (*enemy)->modelScale = 1.0f;
        (*enemy)->enemyModel.setFogUniforms(fogColor, fogStart, fogEnd);
    }

    // Game clock
    *gameClock = new GameClock();

    // Third person camera
    *camController = new ThirdPersonCamera();
}

void cleanupWorld(Scene *scene, TerrainGenerator *terrainGen, TerrainChunkManager *chunkManager,
                  Player *player, Enemy *enemy, GameClock *gameClock,
                  ThirdPersonCamera *camController)
{
    delete camController;
    delete gameClock;
    delete enemy;
    delete player;
    delete chunkManager;
    delete terrainGen;
    delete scene;
}
