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
    
    // Scale the enemy
    transform = glm::scale(transform, glm::vec3(modelScale));
    
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

void Enemy::update(float dt, const Player* player, TerrainChunkManager* terrain)
{
    if (!player) return;
    
    // Calculate distance to player
    glm::vec3 toPlayer = player->position - position;
    toPlayer.y = 0.0f;  // Ignore vertical distance
    float distanceToPlayer = glm::length(toPlayer);
    
    // Only chase if player is within detection range
    if (distanceToPlayer < detectionRange && distanceToPlayer > 3.0f)  // Stop when close
    {
        // Get direction to player
        glm::vec3 direction = getDirectionToPlayer(player);
        
        // Move toward player
        float speed = moveSpeed * dt;
        position += direction * speed;
        
        // Update yaw to face the player
        // atan2 gives us the angle in radians, convert to degrees
        yaw = glm::degrees(atan2(direction.x, direction.z));
    }
    
    // Stick to terrain height
    if (terrain)
    {
        float terrainY = terrain->getPreciseHeightAt(position.x, position.z);
        position.y = terrainY;
    }
}
