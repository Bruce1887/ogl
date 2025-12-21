#pragma once
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
#include "game/EnemySpawner.h"

#include <memory>
#include <glm/glm.hpp>
#include <iostream>
#include <random>

/**
 * @brief Manages all world-related systems including terrain, entities, and rendering
 * 
 * This class encapsulates the game world state and provides a clean interface
 * for initialization, updating, and rendering the world.
 */
class WorldManager {
public:
    WorldManager() = default;
    ~WorldManager() = default;

    // Prevent copying
    WorldManager(const WorldManager&) = delete;
    WorldManager& operator=(const WorldManager&) = delete;

    /**
     * @brief Initialize the game world
     * @return true if initialization successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Update all world systems
     * @param dt Delta time in seconds
     * @param input Input manager reference
     */
    void update(float dt, InputManager *input);
    
    /**
     * @brief Render the world (terrain, entities, skybox)
     */
    void render();
    
    // Getters for UI and other systems
    Player* getPlayer() const { return m_player.get(); }
    Scene* getScene() const { return m_scene.get(); }
    TerrainChunkManager* getChunkManager() const { return m_chunkManager.get(); }
    ThirdPersonCamera* getCameraController() const { return m_camController.get(); }
    
    // Configuration
    void setRenderDistance(float distance);
    float getRenderDistance() const { return m_renderDistance; }
    void setFogColor(const glm::vec3& color) { m_fogColor = color; }
    glm::vec3 getFogColor() const { return m_fogColor; }
    
private:
    // World state - using smart pointers for automatic memory management
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<TerrainGenerator> m_terrainGen;
    std::unique_ptr<TerrainChunkManager> m_chunkManager;
    std::unique_ptr<Player> m_player;
    std::unique_ptr<EnemySpawner> m_enemySpawner;
    // std::unique_ptr<GameClock> m_gameClock;
    std::unique_ptr<ThirdPersonCamera> m_camController;
    
    // Configuration
    float m_renderDistance = 100.0f;
    glm::vec3 m_fogColor = glm::vec3(0.51f, 0.90f, 0.95f);
    
    // Initialization helpers
    bool initializeCamera();
    bool initializeTerrain();
    bool initializeEntities();
    void updateFogSettings();
};