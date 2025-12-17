#include "WorldManager.h"
#include "Common.h"
#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "Terrain/TerrainGenerator.h"
#include "Terrain/TerrainChunk.h"
#include "ThirdPersonCamera.h"
#include "Skybox.h"
#include "game/Player.h"
#include "game/Enemy.h"
#include "game/GameClock.h"

#include <iostream>
#include <random>
#include <stdexcept>

WorldManager::WorldManager() = default;

WorldManager::~WorldManager() {
    cleanup();
}

bool WorldManager::initialize() {
    try {
        if (!initializeCamera()) {
            DEBUG_PRINT("Failed to initialize camera");
            return false;
        }
        
        if (!initializeTerrain()) {
            DEBUG_PRINT("Failed to initialize terrain");
            return false;
        }
        
        if (!initializeEntities()) {
            DEBUG_PRINT("Failed to initialize entities");
            return false;
        }
        
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_PRINT("World initialization failed with exception: " << e.what());
        cleanup();
        return false;
    }
}

bool WorldManager::initializeCamera() {
    // Camera setup
    CameraConfiguration camConfig{
        .fov = 45.0f,
        .aspect = (float)window_X / (float)window_Y,
        .near = 0.1f,
        .far = 2000.0f
    };
    
    Camera camera(camConfig);
    camera.m_Position = glm::vec3(100.0f, 80.0f, 100.0f);
    camera.m_Target = glm::vec3(-50.0f, 60.0f, -50.0f);
    camera.m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // Lighting configuration
    PhongLightConfig lightCfg{
        .lightPosition = glm::vec3(200.0f, 300.0f, 200.0f),
        .ambientLight = glm::vec3(0.4f, 0.4f, 0.45f),
        .diffuseLight = glm::vec3(0.9f, 0.9f, 0.8f),
        .specularLight = glm::vec3(0.5f, 0.5f, 0.5f)
    };
    
    LightSource lightSource = LightSource::fromConfig(lightCfg);
    m_scene = std::make_unique<Scene>(camera, std::move(lightSource));
    
    // Create skybox
    std::vector<std::filesystem::path> skyboxFaces = {
        TEXTURE_DIR / "skybox" / "right.jpg",
        TEXTURE_DIR / "skybox" / "left.jpg",
        TEXTURE_DIR / "skybox" / "top.jpg",
        TEXTURE_DIR / "skybox" / "bottom.jpg",
        TEXTURE_DIR / "skybox" / "front.jpg",
        TEXTURE_DIR / "skybox" / "back.jpg"
    };
    
    m_scene->m_skybox = std::make_unique<Skybox>(skyboxFaces);
    
    // Create camera controller
    m_camController = std::make_unique<ThirdPersonCamera>();
    
    return true;
}

bool WorldManager::initializeTerrain() {
    // Create terrain generator
    m_terrainGen = std::make_unique<TerrainGenerator>();
    
    // Create and compile terrain shader
    auto terrainShader = std::make_shared<Shader>();
    terrainShader->addShader("Terrain.vert", ShaderType::VERTEX);
    terrainShader->addShader("TerrainBlend.frag", ShaderType::FRAGMENT);
    terrainShader->createProgram();
    
    // Load terrain textures
    auto groundTex = Texture::CreateTexture2D(TEXTURE_DIR / "ground.jpg", "u_texture0");
    auto grassTex = Texture::CreateTexture2D(TEXTURE_DIR / "grass.jpg", "u_texture1");
    auto mountainTex = Texture::CreateTexture2D(TEXTURE_DIR / "mountain.jpg", "u_texture2");
    auto blueWaterTex = Texture::CreateTexture2D(TEXTURE_DIR / "blueWater.jpg", "u_texture3");
    auto whiteWaterTex = Texture::CreateTexture2D(TEXTURE_DIR / "whiteWater.jpg", "u_texture4");
    
    std::vector<std::shared_ptr<Texture>> terrainTextures = {
        groundTex, grassTex, mountainTex, blueWaterTex, whiteWaterTex
    };
    
    // Create chunk manager
    m_chunkManager = std::make_unique<TerrainChunkManager>(m_terrainGen.get(), terrainTextures);
    m_chunkManager->setShader(terrainShader);
    
    // Setup fog
    updateFogSettings();
    
    m_camController->setMinHeightFunction([this](float x, float z) {
        return m_chunkManager->getPreciseHeightAt(x, z) + 2.0f;
    });
    return true;
}

bool WorldManager::initializeEntities() {
    // Create player
    m_player = std::make_unique<Player>(
        glm::vec3(100, 0, 100),
        (MODELS_DIR / "cow" / "cow.obj").string()
    );
    
    float fogStart = m_renderDistance * 0.90f;
    float fogEnd = m_renderDistance * 0.98f;
    m_player->m_playerModel.setFogUniforms(m_fogColor, fogStart, fogEnd);
    
    // Spawn enemy near player at random position
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> distanceDist(20.0f, 40.0f);
    
    float spawnAngle = glm::radians(angleDist(gen));
    float spawnDistance = distanceDist(gen);
    glm::vec3 enemySpawnPos = m_player->position + glm::vec3(
        cos(spawnAngle) * spawnDistance,
        0.0f,
        sin(spawnAngle) * spawnDistance
    );
    
    m_enemy = std::make_unique<Enemy>(
        enemySpawnPos,
        (MODELS_DIR / "cow" / "cow.obj").string()
    );
    m_enemy->modelYOffset = 1.0f;
    m_enemy->modelScale = 1.0f;
    m_enemy->enemyModel.setFogUniforms(m_fogColor, fogStart, fogEnd);
    
    // Create game clock
    m_gameClock = std::make_unique<GameClock>();
    
    return true;
}

void WorldManager::update(float dt, InputManager *input) {
    if (!m_scene || !m_player) {
        return;
    }
    
    // Update game clock
    if (m_gameClock) {
        m_gameClock->Update(dt);
    }
    
    // Update player
    if (m_player && m_chunkManager) {
        m_player->update(dt, input, m_chunkManager.get());
    }
    
    // Update enemy
    if (m_enemy && m_player && m_chunkManager) {
        m_enemy->update(dt, m_player.get(), m_chunkManager.get());
    }
    
    // Update camera
    if (m_camController) {
        m_camController->update(m_scene->m_activeCamera, *m_player, dt);
    }
    
    // Update terrain chunks based on camera position
    if (m_chunkManager) {
        m_chunkManager->updateChunks(m_scene->m_activeCamera.m_Position);
    }
}

void WorldManager::render() {
    if (!m_scene) {
        return;
    }
    
    // Enable 3D rendering state
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    
    // Render player
    if (m_player) {
        m_player->render(
            m_scene->m_activeCamera.getViewMatrix(),
            m_scene->m_activeCamera.getProjectionMatrix(),
            &m_scene->m_lightSource.config
        );
    }
    
    // Render enemy
    if (m_enemy) {
        m_enemy->render(
            m_scene->m_activeCamera.getViewMatrix(),
            m_scene->m_activeCamera.getProjectionMatrix(),
            &m_scene->m_lightSource.config
        );
    }
    
    // Render terrain chunks
    if (m_chunkManager) {
        for (const auto& chunk : m_chunkManager->m_chunks) {
            chunk->render(
                m_scene->m_activeCamera.getViewMatrix(),
                m_scene->m_activeCamera.getProjectionMatrix(),
                &m_scene->m_lightSource.config
            );
        }
        
        // Render global water
        m_chunkManager->renderWater(
            m_scene->m_activeCamera.getViewMatrix(),
            m_scene->m_activeCamera.getProjectionMatrix(),
            &m_scene->m_lightSource.config,
            m_scene->m_activeCamera.m_Position,
            m_renderDistance
        );
        
        // Render trees (instanced)
        m_chunkManager->renderTrees(
            m_scene->m_activeCamera.getViewMatrix(),
            m_scene->m_activeCamera.getProjectionMatrix(),
            &m_scene->m_lightSource.config
        );
    }
    
    // Render skybox and scene effects
    m_scene->renderScene();
}

void WorldManager::cleanup() {
    // Smart pointers automatically clean up in reverse order
    m_camController.reset();
    m_gameClock.reset();
    m_enemy.reset();
    m_player.reset();
    m_chunkManager.reset();
    m_terrainGen.reset();
    m_scene.reset();
}

void WorldManager::setRenderDistance(float distance) {
    m_renderDistance = distance;
    updateFogSettings();
}

void WorldManager::updateFogSettings() {
    if (!m_chunkManager) {
        return;
    }
    
    float fogStart = m_renderDistance * 0.90f;
    float fogEnd = m_renderDistance * 0.98f;
    
    // Update chunk manager fog settings
    m_chunkManager->setFogUniforms(m_fogColor, fogStart, fogEnd);
    
    // Update entity fog settings if they exist
    if (m_player) {
        m_player->m_playerModel.setFogUniforms(m_fogColor, fogStart, fogEnd);
    }
    
    if (m_enemy) {
        m_enemy->enemyModel.setFogUniforms(m_fogColor, fogStart, fogEnd);
    }
}