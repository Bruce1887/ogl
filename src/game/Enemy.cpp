#include "Enemy.h"
#include "../Terrain/TerrainChunk.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Enemy::Enemy(glm::vec3 startPos, const std::string& modelPath)
    : position(startPos), enemyModel(modelPath)
{
}

Enemy::Enemy(glm::vec3 startPos, const Model* sharedModel)
    : position(startPos), enemyModel(Model::copyFrom(sharedModel))
{
}

void Enemy::render(glm::mat4 view, glm::mat4 proj, PhongLightConfig* light)
{
    // Build model transform
    glm::mat4 transform(1.0f);
    
    // Translate to world position, adding Y offset so feet touch ground
    glm::vec3 renderPos = position;
    renderPos.y += modelYOffset;
    transform = glm::translate(transform, renderPos);
    
    // Rotate by yaw so model faces forward direction
    transform = glm::rotate(transform, glm::radians(yaw), glm::vec3(0, 1, 0));
    
    // Apply hit wobble rotation when damaged
    if (m_hitFlashTimer > 0.0f)
    {
        float wobble = sin(m_hitFlashTimer * 30.0f) * 0.2f * m_hitFlashTimer;
        transform = glm::rotate(transform, wobble, glm::vec3(0, 0, 1));
    }
    
    // Scale the enemy (with hit pulse effect)
    float currentScale = modelScale + m_hitScaleBoost;
    transform = glm::scale(transform, glm::vec3(currentScale));
    
    enemyModel.setTransform(transform);
    enemyModel.render(view, proj, light);
}

glm::vec3 Enemy::getDirectionToPlayer(const Player* player) const
{
    // Calculate vector from enemy to player
    glm::vec3 toPlayer = player->position - position;
    
    // We only care about horizontal movement (X and Z), not Y
    toPlayer.y = 0.0f;
    
    // Normalize to get direction
    float distance = glm::length(toPlayer);
    if (distance > 0.0001f)  // Avoid division by zero
    {
        return toPlayer / distance;
    }
    
    return glm::vec3(0.0f);
}

void Enemy::update(float dt, const Player* enemy, TerrainChunkManager* terrain)
{
    if (!enemy) return;
    
    // Update visual feedback timers
    if (m_hitFlashTimer > 0.0f)
    {
        m_hitFlashTimer -= dt;
        if (m_hitFlashTimer < 0.0f)
            m_hitFlashTimer = 0.0f;
    }
    if (m_hitScaleBoost > 0.0f)
    {
        m_hitScaleBoost -= dt * 0.2f;  // Decay scale boost over time
        if (m_hitScaleBoost < 0.0f)
            m_hitScaleBoost = 0.0f;
    }
    
    // Update movement timer
    m_movementTimer += dt;
    
    // Calculate distance to player
    glm::vec3 toPlayer = enemy->position - position;
    toPlayer.y = 0.0f;  // Ignore vertical distance
    float distanceToPlayer = glm::length(toPlayer);
    
    // Only chase if player is within detection range
    if (distanceToPlayer < detectionRange && distanceToPlayer > 3.0f)  // Stop when close
    {
        // Get direction to player
        glm::vec3 direction = getDirectionToPlayer(enemy);
        
        // Apply movement pattern
        glm::vec3 moveDirection = direction;
        float currentSpeed = moveSpeed;
        
        switch (movementPattern)
        {
            case MovementPattern::DIRECT:
                // Move straight toward player (default behavior)
                break;
                
            case MovementPattern::ZIGZAG:
            {
                // Calculate perpendicular direction for zigzag
                glm::vec3 perpendicular(-direction.z, 0.0f, direction.x);
                
                // Oscillate left and right using sine wave
                float zigzagOffset = sin(m_movementTimer * zigzagFrequency) * zigzagAmplitude;
                moveDirection = glm::normalize(direction + perpendicular * zigzagOffset * 0.3f);
                break;
            }
                
            case MovementPattern::CAUTIOUS:
                // Move slower overall
                currentSpeed *= 0.7f;
                break;
        }
        
        // Move toward player with pattern applied
        float speed = currentSpeed * dt;
        position += moveDirection * speed;
        
        // Update yaw to face the movement direction
        yaw = glm::degrees(atan2(moveDirection.x, moveDirection.z));
    }
    
    // Stick to terrain height
    if (terrain)
    {
        float terrainY = terrain->getPreciseHeightAt(position.x, position.z);
        position.y = terrainY;
    }
}

void Enemy::takeDamage(float amount)
{
    health -= amount;
    if (health < 0.0f)
        health = 0.0f;
    
    // Trigger hit visual feedback
    m_hitFlashTimer = 0.3f;      // Flash for 0.3 seconds
    m_hitScaleBoost = 0.05f;     // Pulse up 5% in scale
}

bool Enemy::isDead() const
{
    return health <= 0.0f;
}

void Enemy::respawn(glm::vec3 nearPosition, float minDist, float maxDist)
{
    // Generate random angle and distance
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
    float dist = minDist + static_cast<float>(rand()) / RAND_MAX * (maxDist - minDist);
    
    // Set new position
    position = nearPosition + glm::vec3(cos(angle) * dist, 0.0f, sin(angle) * dist);
    
    // Reset health
    health = maxHealth;
    
    // Reset movement timer
    m_movementTimer = 0.0f;
    
    // Reset visual effects
    m_hitFlashTimer = 0.0f;
    m_hitScaleBoost = 0.0f;
}
