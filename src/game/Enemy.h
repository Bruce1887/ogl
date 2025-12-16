#pragma once
#include <glm/glm.hpp>
#include "../Model.h"
#include "Player.h"

class TerrainChunkManager;

// Movement pattern types
enum class MovementPattern
{
    DIRECT,     // Moves straight toward player
    ZIGZAG,     // Moves in a zigzag pattern toward player
    CAUTIOUS    // Moves slower, stops occasionally
};

class Enemy
{
public:
    glm::vec3 position;
    float yaw = 0.0f;
    
    // Health system
    float health = 100.0f;
    float maxHealth = 100.0f;
    
    float moveSpeed = 8.0f;  // Base movement speed
    float detectionRange = 100.0f;  // Range at which enemy detects player
    float modelYOffset = 20.0f;  // Vertical offset to place feet on ground (adjust per model)
    float modelScale = 0.01f;   // Scale of the model
    
    // Movement behavior
    MovementPattern movementPattern = MovementPattern::DIRECT;
    float zigzagAmplitude = 5.0f;   // How far the zigzag goes left/right
    float zigzagFrequency = 3.0f;   // How fast the zigzag oscillates
    
    Model enemyModel;
    
    // Constructor that loads its own model (expensive)
    Enemy(glm::vec3 startPos, const std::string& modelPath);
    
    // Constructor that shares model data (efficient - use for multiple enemies)
    Enemy(glm::vec3 startPos, const Model* sharedModel);
    
    // Update enemy AI: track player and move toward them
    void update(float dt, const Player* player, TerrainChunkManager* terrain);
    
    void render(glm::mat4 view, glm::mat4 proj, PhongLightConfig* light);
    
    // Health methods
    void takeDamage(float amount);
    bool isDead() const;
    
    // Respawn at a random position near the given point
    void respawn(glm::vec3 nearPosition, float minDist = 10.0f, float maxDist = 30.0f);
    
private:
    // Calculate direction to player
    glm::vec3 getDirectionToPlayer(const Player* player) const;
    
    // Internal timer for movement patterns
    float m_movementTimer = 0.0f;
    
    // Visual feedback
    float m_hitFlashTimer = 0.0f;    // Time remaining for hit flash effect
    float m_hitScaleBoost = 0.0f;    // Extra scale when hit (pulses down)
};
