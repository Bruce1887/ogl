#pragma once
#include <glm/glm.hpp>

#include "Enums.h"
// Movement pattern types
enum class MovementPattern
{
	DIRECT,	 // Moves straight toward player
	ZIGZAG,	 // Moves in a zigzag pattern toward player
	CAUTIOUS // Moves slower, stops occasionally
};

/**
 * @brief Data structure holding all relevant data for an enemy apart from its model and rendering
 *
 */
struct EnemyData
{
	// Transform-related
	glm::vec3 m_position;
	float m_yaw = 0.0f;

	int killScore = 1; // Points awarded to player for killing this enemy

	// Health system
	float m_health = 100.0f;
	float m_maxHealth = 100.0f;

	// Attack properties
	float m_attackDamage = 5.0f;   // Damage dealt to player on attack
	float m_attackCooldown = 4.0f; // Seconds between attacks
	float m_attackTimer = 0.0f;	   // Tracks time since last attack
	// float m_attackRange = 2.0f;    // Range within which enemy can attack player (this is implied by m_closeRange)

	// Movement behavior
	float m_moveSpeed = 6.0f; // Base movement speed
	MovementPattern m_movementPattern = MovementPattern::DIRECT;
	float m_zigzagAmplitude = 5.0f;								// How far the zigzag goes left/right
	float m_zigzagFrequency = 3.0f;								// How fast the zigzag oscillates
	float m_detectionRange = std::numeric_limits<float>::max(); // Range at which enemy detects player (default: infinite)
	float m_closeRange = 2.0f;									// Range at which enemy stops moving toward player (attack range)

	// Internal timer for movement patterns
	float m_movementTimer = 0.0f;

	// Visual feedback
	float m_hitFlashTimer = 0.0f; // Time remaining for hit flash effect
	float m_hitScaleBoost = 0.0f; // Extra scale when hit (pulses down)

	float m_modelYOffset = 0.0f; // Vertical offset to place feet on ground (adjust per model)
	float m_modelScale = 1.0f;	 // Scale of the model (dont think this works)

	// Calculate direction to target position on the XZ plane
	glm::vec3 getDirectionXZtoTarget(const glm::vec3 &targetPos) const
	{
		// Calculate vector from enemy to target
		glm::vec3 toTarget = targetPos - m_position;

		// We only care about horizontal movement (X and Z), not Y
		toTarget.y = 0.0f;

		// Normalize to get direction
		float distance = glm::length(toTarget);
		if (distance > 0.0001f) // Avoid division by zero
		{
			return toTarget / distance;
		}

		return glm::vec3(0.0f);
	}

	void updateTimers(float dt)
	{
		// Time since last attack
		m_attackTimer += dt;
		// Movement pattern timer
		m_movementTimer += dt;

		// Visual feedback timers
		if (m_hitFlashTimer > 0.0f)
		{
			m_hitFlashTimer -= dt;
			if (m_hitFlashTimer < 0.0f)
				m_hitFlashTimer = 0.0f;
		}
		if (m_hitScaleBoost > 0.0f)
		{
			m_hitScaleBoost -= dt * 0.2f; // Decay scale boost over time
			if (m_hitScaleBoost < 0.0f)
				m_hitScaleBoost = 0.0f;
		}
	}

	bool isDead() const
	{
		return m_health <= 0.0f;
	}

	void takeDamage(float amount)
	{
		m_health -= amount;
		if (m_health < 0.0f)
			m_health = 0.0f;

		// Trigger hit visual feedback
		m_hitFlashTimer = 0.3f;	 // Flash for 0.3 seconds
		m_hitScaleBoost = 0.05f; // Pulse up 5% in scale
	}

	// tries to return attack damage if not too soon since last attack. Applying damage to player is handled elsewhere.
	float tryAttack(float dt)
	{
		m_attackTimer += dt;
		if (m_attackTimer < m_attackCooldown)
		{

			return 0.0f; // Attack is on cooldown
		}
		m_attackTimer = 0.0f; // Reset attack timer after successful attack
		// For now, always return damage (attack logic handled elsewhere)
		return m_attackDamage;
	}

	void setAnimationState(AnimationState state)
	{
		if (m_animation_lock)
			return;
		m_animationState = state;
	}

	AnimationState getAnimationState() const
	{
		return m_animationState;
	}

	void lockAnimationState(bool lock)
	{
		m_animation_lock = lock;
	}
private:
	AnimationState m_animationState = AnimationState::IDLE; // Current animation state
	bool m_animation_lock = false;							// If true, animation state won't change automatically
};