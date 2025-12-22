#include "Enemy.h"
#include "../Terrain/TerrainChunk.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Enemy::Enemy(EnemyData enemyData, const std::string &modelPath)
    : m_enemyData(enemyData), m_enemyModel(modelPath)
{
}

Enemy::Enemy(EnemyData enemyData, const Model *sharedModel)
    : m_enemyData(enemyData), m_enemyModel(Model::copyFrom(sharedModel))
{
}

void Enemy::render(glm::mat4 view, glm::mat4 proj, PhongLightConfig *light)
{
    // Build model transform
    glm::mat4 transform(1.0f);

    // Translate to world position, adding Y offset so feet touch ground
    glm::vec3 renderPos = m_enemyData.m_position;
    renderPos.y += m_enemyData.m_modelYOffset;
    transform = glm::translate(transform, renderPos);

    // Rotate by yaw so model faces forward direction
    transform = glm::rotate(transform, glm::radians(m_enemyData.m_yaw), glm::vec3(0, 1, 0));

    // Apply hit wobble rotation when damaged
    if (m_enemyData.m_hitFlashTimer > 0.0f)
    {
        float wobble = sin(m_enemyData.m_hitFlashTimer * 30.0f) * 0.2f * m_enemyData.m_hitFlashTimer;
        transform = glm::rotate(transform, wobble, glm::vec3(0, 0, 1));
    }

    // Scale the enemy (with hit pulse effect)
    float currentScale = m_enemyData.m_modelScale + m_enemyData.m_hitScaleBoost;
    transform = glm::scale(transform, glm::vec3(currentScale));

    m_enemyModel.setTransform(transform);
    m_enemyModel.render(view, proj, light);
}

void Enemy::update(float dt, const glm::vec3& playerPosition, TerrainChunkManager *terrain)
{
    // Update visual feedback timers
    if (m_enemyData.m_hitFlashTimer > 0.0f)
    {
        m_enemyData.m_hitFlashTimer -= dt;
        if (m_enemyData.m_hitFlashTimer < 0.0f)
            m_enemyData.m_hitFlashTimer = 0.0f;
    }
    if (m_enemyData.m_hitScaleBoost > 0.0f)
    {
        m_enemyData.m_hitScaleBoost -= dt * 0.2f; // Decay scale boost over time
        if (m_enemyData.m_hitScaleBoost < 0.0f)
            m_enemyData.m_hitScaleBoost = 0.0f;
    }

    // Update movement timer
    m_enemyData.m_movementTimer += dt;

    // Calculate distance to player
    glm::vec3 toPlayer = playerPosition - m_enemyData.m_position;
    toPlayer.y = 0.0f; // Ignore vertical distance
    float distanceToPlayer = glm::length(toPlayer);

    // Only chase if player is within detection range
    if (distanceToPlayer < m_enemyData.m_detectionRange && distanceToPlayer > 3.0f) // Stop when close
    {
        // Get direction to player
        glm::vec3 direction = m_enemyData.getDirectionXZtoTarget(playerPosition);

        // Apply movement pattern
        glm::vec3 moveDirection = direction;
        float currentSpeed = m_enemyData.m_moveSpeed;
        switch (m_enemyData.m_movementPattern)
        {
        case MovementPattern::DIRECT:
            // Move straight toward player (default behavior)
            break;

        case MovementPattern::ZIGZAG:
        {
            // Calculate perpendicular direction for zigzag
            glm::vec3 perpendicular(-direction.z, 0.0f, direction.x);

            // Oscillate left and right using sine wave
            float zigzagOffset = sin(m_enemyData.m_movementTimer * m_enemyData.m_zigzagFrequency) * m_enemyData.m_zigzagAmplitude;
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
        m_enemyData.m_position += moveDirection * speed;

        // Update yaw to face the movement direction
        m_enemyData.m_yaw = glm::degrees(atan2(moveDirection.x, moveDirection.z));
    }

    // Stick to terrain height
    if (terrain)
    {
        float terrainY = terrain->getPreciseHeightAt(m_enemyData.m_position.x, m_enemyData.m_position.z);
        m_enemyData.m_position.y = terrainY;
    }
}

void Enemy::takeDamage(float amount)
{
    m_enemyData.takeDamage(amount);
}

bool Enemy::isDead() const
{
    return m_enemyData.isDead();
}

void Enemy::respawn(glm::vec3 nearPosition, float minDist, float maxDist)
{
    // Generate random angle and distance
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159f;
    float dist = minDist + static_cast<float>(rand()) / RAND_MAX * (maxDist - minDist);

    // Set new position
    m_enemyData.m_position = nearPosition + glm::vec3(cos(angle) * dist, 0.0f, sin(angle) * dist);

    // Reset health
    m_enemyData.m_health = m_enemyData.m_maxHealth;

    // Reset movement timer
    m_enemyData.m_movementTimer = 0.0f;

    // Reset visual effects
    m_enemyData.m_hitFlashTimer = 0.0f;
    m_enemyData.m_hitScaleBoost = 0.0f;
}
