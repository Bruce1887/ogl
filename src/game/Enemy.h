#pragma once
#include <glm/glm.hpp>
#include "../Model.h"
#include "Player.h"
#include "EnemyData.h"

class TerrainChunkManager;

class Enemy
{
public:
    EnemyData m_enemyData;

    Model m_enemyModel;

    // Constructor that loads its own model (expensive)
    Enemy(EnemyData enemyData, const std::string &modelPath);

    // Constructor that shares model data (efficient - use for multiple enemies)
    Enemy(EnemyData enemyData, const Model *sharedModel);

    // Update enemy AI: track player and move toward them
    void update(float dt, const glm::vec3& playerPosition, TerrainChunkManager *terrain);

    void render(glm::mat4 view, glm::mat4 proj, PhongLightConfig *light);

    // Health methods
    void takeDamage(float amount);
    bool isDead() const;

    // Respawn at a random position near the given point
    void respawn(glm::vec3 nearPosition, float minDist = 10.0f, float maxDist = 30.0f);
};
