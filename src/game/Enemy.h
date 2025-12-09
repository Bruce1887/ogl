#pragma once
#include <glm/glm.hpp>
#include "../Model.h"
#include "../Player.h"

class TerrainChunkManager;

class Enemy
{
public:
    glm::vec3 position;
    float yaw = 0.0f;
    
    float moveSpeed = 5.0f;  // Slower than player
    float detectionRange = 30.0f;  // Range at which enemy detects player
    float modelYOffset = 20.0f;  // Vertical offset to place feet on ground (adjust per model)
    float modelScale = 0.01f;   // Scale of the model
    
    Model enemyModel;
    
    // Constructor that loads its own model (expensive)
    Enemy(glm::vec3 startPos, const std::string& modelPath);
    
    // Constructor that shares model data (efficient - use for multiple enemies)
    Enemy(glm::vec3 startPos, const Model* sharedModel);
    
    // Update enemy AI: track player and move toward them
    void update(float dt, const Player* player, TerrainChunkManager* terrain);
    
    void render(glm::mat4 view, glm::mat4 proj, PhongLightConfig* light);
    
private:
    // Calculate direction to player
    glm::vec3 getDirectionToPlayer(const Player* player) const;
};
