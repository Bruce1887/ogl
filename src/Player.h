#pragma once
#include <glm/glm.hpp>
#include "Model.h"
#include <vector>

class TerrainChunkManager;
class InputManager;
class Enemy;

class Player
{
public:
    glm::vec3 position;
    float yaw = 0.0f;

    float moveSpeed = 10.0f;
    float modelScale = 1.0f;
    float modelYOffset = 0.0f;
    
    // Attack properties
    float attackRange = 5.0f;
    float attackDamage = 25.0f;
    float attackCooldown = 0.1f;  // seconds between attacks

    Model playerModel;

    Player(glm::vec3 startPos, const std::string& modelPath);
    void update(float dt, InputManager* input, TerrainChunkManager* terrain);
    
    // Attack all enemies within attackRange, returns number of enemies hit
    int attack(std::vector<Enemy*>& enemies);

    void render(glm::mat4 view, glm::mat4 proj, PhongLightConfig* light);
    
private:
    float m_attackTimer = 0.0f;  // tracks cooldown
};
