#pragma once
#include <glm/glm.hpp>
#include "Model.h"
#include <vector>

class TerrainChunkManager;
class InputManager;
class EnemyData;


class Player
{
public:
    glm::vec3 m_position;
    float m_yaw = 0.0f;

    float m_moveSpeed = 10.0f;
    float m_modelScale = 1.0f;
    float m_modelYOffset = 0.0f;
    float m_rotationSpeed = 90.0f;
    
    // Attack properties
    float m_attackRange = 5.0f;
    float m_attackDamage = 25.0f;
    float m_attackCooldown = 0.1f;  // seconds between attacks

    Model m_playerModel;

    Player(glm::vec3 startPos, const std::string& modelPath);
    void update(float dt, InputManager* input, TerrainChunkManager* terrain);
    
    // Attack all enemies within attackRange, returns number of enemies hit
    int attack(std::vector<EnemyData>& enemies);

    void render(glm::mat4 view, glm::mat4 proj, PhongLightConfig* light);
    
private:
    float m_attackTimer = 0.0f;  // tracks cooldown
};
