#include "Player.h"
#include "Terrain/TerrainChunk.h"
#include "Input/UserInput.h"
#include "game/Enemy.h"
#include <glm/gtc/matrix_transform.hpp>



Player::Player(glm::vec3 startPos, const std::string& modelPath)
    : position(startPos), playerModel(modelPath)
{
}


void Player::render(glm::mat4 view, glm::mat4 proj, PhongLightConfig* light)
{
    // Build model transform
    glm::mat4 transform(1.0f);

    // translate to world position (with Y offset for proper ground placement)
    transform = glm::translate(transform, position + glm::vec3(0.0f, modelYOffset, 0.0f));

    // rotate by yaw so model faces forward direction
    transform = glm::rotate(transform, glm::radians(yaw), glm::vec3(0,1,0));

    // scale the model
    transform = glm::scale(transform, glm::vec3(modelScale));

    playerModel.setTransform(transform);
    playerModel.render(view, proj, light);
}


void Player::update(float dt, InputManager* input, TerrainChunkManager* terrain)
{
    int forwardMove, rightMove;
    bool shiftDown;
    input->movementInput.fetchMovement(forwardMove, rightMove, shiftDown);

    float speed = moveSpeed * dt;
    if (shiftDown) speed *= 2.0f;

    // derive forward vector from yaw
    glm::vec3 forward(
        sin(glm::radians(yaw)),
        0,
        cos(glm::radians(yaw))
    );

    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));

    // movement
    glm::vec3 movement(0.0f);

    if (forwardMove != 0)
        movement += forward * (forwardMove * speed);

    if (rightMove != 0)
        movement += right * (rightMove * speed);

    position += movement;

    // collision with terrain
    float terrainY = terrain->getPreciseHeightAt(position.x, position.z);
    position.y = terrainY;

    // --- Turn player left/right ---
    if (glfwGetKey(g_window, GLFW_KEY_Q) == GLFW_PRESS)
        yaw += 60.0f * dt;

    if (glfwGetKey(g_window, GLFW_KEY_E) == GLFW_PRESS)
        yaw -= 60.0f * dt;

    // Update attack cooldown timer
    if (m_attackTimer > 0.0f)
        m_attackTimer -= dt;
}

int Player::attack(std::vector<Enemy*>& enemies)
{
    // Check cooldown
    if (m_attackTimer > 0.0f)
        return 0;
    
    // Reset cooldown
    m_attackTimer = attackCooldown;
    
    int enemiesHit = 0;
    
    // Check all enemies and damage those within range
    for (Enemy* enemy : enemies)
    {
        if (enemy->isDead())
            continue;
            
        // Calculate distance to enemy (XZ plane only)
        glm::vec3 toEnemy = enemy->position - position;
        toEnemy.y = 0.0f;
        float distance = glm::length(toEnemy);
        
        if (distance <= attackRange)
        {
            enemy->takeDamage(attackDamage);
            enemiesHit++;
        }
    }
    
    return enemiesHit;
}
