#include <glm/glm.hpp>

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
	
	// Health system
	float m_health = 100.0f;
	float m_maxHealth = 100.0f;

	// Movement behavior
	float m_moveSpeed = 8.0f;		   // Base movement speed
	MovementPattern m_movementPattern = MovementPattern::DIRECT;
	float m_zigzagAmplitude = 5.0f; // How far the zigzag goes left/right
	float m_zigzagFrequency = 3.0f; // How fast the zigzag oscillates
	
	float m_detectionRange = 100.0f; // Range at which enemy detects player

	// Calculate direction to target position on the XZ plane
	glm::vec3 getDirectionXZtoTarget(const glm::vec3& targetPos) const
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

	// Internal timer for movement patterns
	float m_movementTimer = 0.0f;

	// Visual feedback
	float m_hitFlashTimer = 0.0f; // Time remaining for hit flash effect
	float m_hitScaleBoost = 0.0f; // Extra scale when hit (pulses down)

	float m_modelYOffset = 0.0f; // Vertical offset to place feet on ground (adjust per model)
    float m_modelScale = 1.0f;   // Scale of the model
};