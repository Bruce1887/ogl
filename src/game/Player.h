#pragma once
#include "Model.h"
#include "AnimatedInstanceRenderer.h"
#include "Score.h"
#include "Audio.h"

#include <glm/glm.hpp>
#include <vector>

class TerrainChunkManager;
class InputManager;
struct EnemyData;

struct PlayerData
{
    glm::vec3 m_position;
    float m_yaw = 0.0f;
    float m_campitch = 0.0f;

    bool m_isGrounded = true;
    float m_verticalVelocity = 0.0f;

    float m_jumpVelocity = 10.0f;
    float m_gravity = 20.0f;

    float m_maxHealth = 100.0f;
    float m_health = m_maxHealth; // Set starting health to max health

    float m_moveSpeed = 10.0f;
    float m_modelScale = 1.0f;
    float m_modelYOffset = 0.0f;
    float m_rotationSpeed = 90.0f;

    // Attack properties
    float m_attackRange = 4.0f;       // Radius of attack-circle
    float m_attackRangeOffset = 4.0f; // how far in front of player the attack-circle is centered
    float m_attackDamage = 25.0f;
    float m_attackCooldown = 0.1f; // seconds between attacks
    float m_attackTimer = 0.0f;    // tracks cooldown

    // Special attack properties (J key)
    float m_specialAttackRange = 25.0f;    // Large AOE range
    float m_specialAttackCooldown = 30.0f; // 30 seconds cooldown
    float m_specialAttackTimer = 0.0f;     // Ready when 0

    bool isSpecialAttackReady() const { return m_specialAttackTimer <= 0.0f; }
    float getSpecialAttackCooldownPercent() const
    {
        return m_specialAttackTimer > 0.0f ? m_specialAttackTimer / m_specialAttackCooldown : 0.0f;
    }

    void take_damage(float damage)
    {
        if (damage >= m_health)
        {
            m_health = 0.0;
        }
        else
        {
            m_health -= damage;
        }
    }

    AnimationState getAnimationState() const
    {
        return m_animationState;
    }

    /**
     * @brief Set the Animation State object
     *
     * @param state The new animation state
     * @param lock If true, locks the animation state to prevent automatic changes
     */
    void setAnimationState(AnimationState state, bool lock = false)
    {
        if (m_animation_lock)
            return;

        if (lock)
            m_animation_lock = true;

        m_animationState = state;
    }

    /**
     * @brief Unlock the animation state if the current state matches the expected state
     *
     * @param expectedState The state that is expected to be current for unlocking to occur
     */
    inline void unlockAnimationState(AnimationState expectedState)
    {
        if (m_animationState != expectedState)
        {
            return;
        }

        m_animation_lock = false;
    }

private:
    AnimationState m_animationState = AnimationState::IDLE; // Current animation state
    bool m_animation_lock = false;                          // If true, animation state won't change automatically
};

class Player
{
public:
    PlayerData m_playerData;

    std::unique_ptr<AnimatedInstanceRenderer> m_playerRenderer;

    Player(PlayerData playerData);
    void update(float dt, InputManager *input, TerrainChunkManager *terrain);

    // Attack all enemies within attackRange, returns number of enemies hit
    int attack(std::vector<EnemyData *> &enemies);

    // Special attack - kills all enemies in range, long cooldown
    int specialAttack(std::vector<EnemyData *> &enemies);

    void render(glm::mat4 view, glm::mat4 proj, PhongLightConfig *light);

    void setEntitySounds(const EntitySounds &sounds)
    {
        m_sounds = sounds;
    }

    // Score access
    unsigned int getScore() const { return m_scoreKeeper.getScore(); }
    void addScore(unsigned int points) { m_scoreKeeper.addPoints(points); }

private:
    ScoreKeeper m_scoreKeeper;
    std::optional<EntitySounds> m_sounds;
};
