#include "Player.h"
#include "Terrain/TerrainChunk.h"
#include "Input/UserInput.h"
#include "game/EnemyData.h"
#include <glm/gtc/matrix_transform.hpp>

Player::Player(glm::vec3 startPos, const std::string &modelPath)
    : m_playerModel(modelPath)
{
    m_playerData.m_position = startPos;
}

void Player::render(glm::mat4 view, glm::mat4 proj, PhongLightConfig *light)
{
    // Build model transform
    glm::mat4 transform(1.0f);

    // translate to world position (with Y offset for proper ground placement)
    transform = glm::translate(transform, m_playerData.m_position + glm::vec3(0.0f, m_playerData.m_modelYOffset, 0.0f));

    // rotate by yaw so model faces forward direction
    transform = glm::rotate(transform, glm::radians(m_playerData.m_yaw), glm::vec3(0, 1, 0));

    // scale the model
    transform = glm::scale(transform, glm::vec3(m_playerData.m_modelScale));

    m_playerModel.setTransform(transform);
    m_playerModel.render(view, proj, light);
}

void Player::update(float dt, InputManager *input, TerrainChunkManager *terrain)
{
    int forwardMove, rightMove;
    bool shiftDown;
    input->keyboardInput.movementInput.fetchMovement(forwardMove, rightMove, shiftDown);

    float speed = m_playerData.m_moveSpeed * dt;
    if (shiftDown)
        speed *= 2.0f;

    // derive forward vector from yaw
    glm::vec3 forward(
        sin(glm::radians(m_playerData.m_yaw)),
        0,
        cos(glm::radians(m_playerData.m_yaw)));

    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

    // movement
    glm::vec3 movement(0.0f);

    if (forwardMove != 0)
        movement += forward * (forwardMove * speed);

    if (rightMove != 0)
        movement += right * (rightMove * speed);

    m_playerData.m_position += movement;

    // collision with terrain
    float terrainY = terrain->getPreciseHeightAt(m_playerData.m_position.x, m_playerData.m_position.z);
    m_playerData.m_position.y = terrainY;

    // --- Turn player left/right ---
    if (glfwGetKey(g_window, GLFW_KEY_Q) == GLFW_PRESS)
        m_playerData.m_yaw += m_playerData.m_rotationSpeed * dt;

    if (glfwGetKey(g_window, GLFW_KEY_E) == GLFW_PRESS)
        m_playerData.m_yaw -= m_playerData.m_rotationSpeed * dt;

    // Update attack cooldown timer
    if (m_playerData.m_attackTimer > 0.0f)
        m_playerData.m_attackTimer -= dt;


}

int Player::attack(std::vector<EnemyData*> &enemies)
{
    // Check cooldown
    if (m_playerData.m_attackTimer > 0.0f)
        return 0;

    // Reset cooldown
    m_playerData.m_attackTimer = m_playerData.m_attackCooldown;

    int enemiesHit = 0;

    // Check all enemies and damage those within range
    for (EnemyData* e_data : enemies)
    {
        if (e_data->isDead())
            continue;

        // Calculate distance to enemy (XZ plane only)
        glm::vec3 toEnemy = e_data->m_position - m_playerData.m_position;
        toEnemy.y = 0.0f;
        float distance = glm::length(toEnemy);

        if (distance <= m_playerData.m_attackRange)
        {
            e_data->takeDamage(m_playerData.m_attackDamage);
            enemiesHit++;

            if(e_data->isDead())
                m_scoreKeeper.addPoints(1); // Award points for kill
        }
    }

    DEBUG_PRINT("player score: " << m_scoreKeeper.getScore());
    return enemiesHit;
}
