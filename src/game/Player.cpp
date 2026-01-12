#include "Player.h"
#include "Terrain/TerrainChunk.h"
#include "Input/UserInput.h"
#include "game/EnemyData.h"
#include <glm/gtc/matrix_transform.hpp>

Player::Player(PlayerData playerData)
    : m_playerData(playerData)
{
    m_playerRenderer = std::make_unique<AnimatedInstanceRenderer>();
}

void Player::render(glm::mat4 view, glm::mat4 proj, PhongLightConfig *light)
{
    m_playerRenderer->render(view, proj, light);
}

static glm::vec2 resolveXZCollisions(
    const glm::vec2 &current,
    const glm::vec2 &proposed,
    float playerRadius,
    float jumpover,
    const std::vector<StaticObstacle> &obstacles)
{
    glm::vec2 result = proposed;

    for (const auto &o : obstacles)
    {
        if (jumpover > 2.0f)
            continue;
        glm::vec2 diff = result - o.posXZ;
        float dist = glm::length(diff);
        float minDist = playerRadius + o.radius;

        if (dist < minDist && dist > 0.0001f)
        {
            glm::vec2 normal = diff / dist;
            result = o.posXZ + normal * minDist;
        }
    }

    return result;
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

    if (forwardMove != 0 || rightMove != 0)
        m_playerData.setAnimationState(AnimationState::WALKING);
    else
        m_playerData.setAnimationState(AnimationState::IDLE);

    // m_playerData.m_position += movement;

    float terrainY = terrain->getPreciseHeightAt(
        m_playerData.m_position.x,
        m_playerData.m_position.z);

    glm::vec3 proposed = m_playerData.m_position + movement;

    std::vector<StaticObstacle> nearbyObstacles;
    terrain->collectNearbyObstacles(m_playerData.m_position, 5.0f, nearbyObstacles);

    // --- resolve XZ collision ---
    glm::vec2 correctedXZ = resolveXZCollisions(
        glm::vec2(m_playerData.m_position.x, m_playerData.m_position.z),
        glm::vec2(proposed.x, proposed.z),
        1.0f,
        m_playerData.m_position.y - terrainY,
        nearbyObstacles);

    m_playerData.m_position.x = correctedXZ.x;
    m_playerData.m_position.z = correctedXZ.y;

    // collision with terrain
    // float terrainY = terrain->getPreciseHeightAt(m_playerData.m_position.x, m_playerData.m_position.z);
    // m_playerData.m_position.y = terrainY;

    // --- jump input ---
    if (glfwGetKey(g_window, GLFW_KEY_SPACE) == GLFW_PRESS && m_playerData.m_isGrounded)
    {
        m_playerData.m_verticalVelocity = m_playerData.m_jumpVelocity;
        m_playerData.m_isGrounded = false;
    }

    // --- gravity ---
    m_playerData.m_verticalVelocity -= m_playerData.m_gravity * dt;

    // --- vertical integration ---
    m_playerData.m_position.y += m_playerData.m_verticalVelocity * dt;

    if (m_playerData.m_position.y <= terrainY)
    {
        m_playerData.m_position.y = terrainY;
        m_playerData.m_verticalVelocity = 0.0f;
        m_playerData.m_isGrounded = true;
    }

    // --- Turn player left/right ---
    if (glfwGetKey(g_window, GLFW_KEY_Q) == GLFW_PRESS)
        m_playerData.m_yaw += m_playerData.m_rotationSpeed * dt;

    if (glfwGetKey(g_window, GLFW_KEY_E) == GLFW_PRESS)
        m_playerData.m_yaw -= m_playerData.m_rotationSpeed * dt;

    // Update attack cooldown timer
    if (m_playerData.m_attackTimer > 0.0f)
    {
        m_playerData.m_attackTimer -= dt;
        if (m_playerData.m_attackTimer < 0.0f)
        {
            m_playerData.m_attackTimer = 0.0f;
            m_playerData.unlockAnimationState(AnimationState::ATTACK);
        }
    }

    // Update special attack cooldown timer
    if (m_playerData.m_specialAttackTimer > 0.0f)
    {
        m_playerData.m_specialAttackTimer -= dt;
    }

    double dx, dy;
    if (input->mouseMoveInput.fetchDeltas(dx, dy))
    {
        float sensitivity = 0.1f;
        m_playerData.m_yaw -= dx * sensitivity;
        m_playerData.m_campitch -= dy * sensitivity * 2 / 3;
        m_playerData.m_campitch = glm::clamp(m_playerData.m_campitch, -90.0f, 90.0f);
    }

    // Build transform
    glm::mat4 transform(1.0f);
    // translate to world position (with Y offset for proper ground placement)
    transform = glm::translate(transform, m_playerData.m_position + glm::vec3(0.0f, m_playerData.m_modelYOffset, 0.0f));
    // rotate by yaw so model faces forward direction
    transform = glm::rotate(transform, glm::radians(m_playerData.m_yaw), glm::vec3(0, 1, 0));
    // scale the model
    transform = glm::scale(transform, glm::vec3(m_playerData.m_modelScale));

    std::unordered_map<AnimationState, std::vector<glm::mat4>> instanceTransformsByState; // super hacky but idc
    instanceTransformsByState[m_playerData.getAnimationState()].push_back(transform);
    m_playerRenderer->updateInstances(instanceTransformsByState, dt);
}

int Player::attack(std::vector<EnemyData *> &enemies)
{
    // Check cooldown
    if (m_playerData.m_attackTimer > 0.0f)
        return 0;

    // Reset cooldown
    m_playerData.m_attackTimer = m_playerData.m_attackCooldown;

    int enemiesHit = 0;

    float attackHeightOffset = 0.8f; // Abbe modellen är ungefär 1.2 - 1.4 units hög, så 0.8 är ungefär vid midja/brösthöjd
    glm::vec3 attack_circle_center = m_playerData.m_position +
                                     glm::vec3(
                                         sin(glm::radians(m_playerData.m_yaw)) * m_playerData.m_attackRangeOffset,
                                         attackHeightOffset,
                                         cos(glm::radians(m_playerData.m_yaw)) * m_playerData.m_attackRangeOffset);

    // Check all enemies and damage those within range
    for (EnemyData *e_data : enemies)
    {
        if (e_data->isDead())
            continue;

        // Calculate distance to enemy (XZ plane only)

        glm::vec3 toEnemy = e_data->m_position - attack_circle_center;
        // toEnemy.y = 0.0f;
        float distance = glm::length(toEnemy);

        if (distance <= m_playerData.m_attackRange)
        {
            e_data->takeDamage(m_playerData.m_attackDamage);
            enemiesHit++;

            if (e_data->isDead())
                m_scoreKeeper.addPoints(e_data->killScore); // Award points for kill
        }
    }

    // Set animation state to ATTACK and play sound if any enemies were hit
    if (enemiesHit > 0)
    {
        m_playerData.setAnimationState(AnimationState::ATTACK, true);
        if (m_sounds.has_value())
            SoundPlayer::getInstance().PlaySFX((*m_sounds).m_attackSound);
    }

    // DEBUG_PRINT("player score: " << m_scoreKeeper.getScore());
    return enemiesHit;
}

int Player::specialAttack(std::vector<EnemyData *> &enemies)
{
    // Check cooldown
    if (m_playerData.m_specialAttackTimer > 0.0f)
    {
        DEBUG_PRINT("Special attack on cooldown: " << m_playerData.m_specialAttackTimer << "s remaining");
        return 0;
    }

    // Reset cooldown
    m_playerData.m_specialAttackTimer = m_playerData.m_specialAttackCooldown;

    int enemiesKilled = 0;

    DEBUG_PRINT("=== SPECIAL ATTACK ACTIVATED ===");

    // Insta-kill all enemies within special attack range
    for (EnemyData *e_data : enemies)
    {
        if (e_data->isDead())
            continue;

        // Calculate distance to enemy (XZ plane only)
        glm::vec3 toEnemy = e_data->m_position - m_playerData.m_position;
        toEnemy.y = 0.0f;
        float distance = glm::length(toEnemy);

        if (distance <= m_playerData.m_specialAttackRange)
        {
            // Insta-kill by dealing massive damage
            e_data->takeDamage(e_data->m_maxHealth * 100.0f);
            enemiesKilled++;
            m_scoreKeeper.addPoints(1);
        }
    }

    DEBUG_PRINT("Special attack killed " << enemiesKilled << " enemies!");
    DEBUG_PRINT("player score: " << m_scoreKeeper.getScore());
    return enemiesKilled;
}