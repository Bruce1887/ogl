#include "EnemySpawner.h"

void EnemySpawner::updateAll(float dt, const glm::vec3 &playerPosition)
{
	float timenow = glfwGetTime();
	if (timenow - m_spawnTimer > m_spawnInterval)
	{
		m_spawnTimer = timenow;
		spawnNew(playerPosition, 20.0f, 40.0f);
	}

	std::vector<glm::mat4> newTransforms;
	newTransforms.reserve(m_enemyDataList.size());

	for (int i = 0; i < m_enemyDataList.size(); i++)
	{
		EnemyData &data = m_enemyDataList[i];

		// Update visual feedback timers
		if (data.m_hitFlashTimer > 0.0f)
		{
			data.m_hitFlashTimer -= dt;
			if (data.m_hitFlashTimer < 0.0f)
				data.m_hitFlashTimer = 0.0f;
		}
		if (data.m_hitScaleBoost > 0.0f)
		{
			data.m_hitScaleBoost -= dt * 0.2f; // Decay scale boost over time
			if (data.m_hitScaleBoost < 0.0f)
				data.m_hitScaleBoost = 0.0f;
		}

		// Update movement timer
		data.m_movementTimer += dt;

		// Calculate distance to player
		glm::vec3 toPlayer = playerPosition - data.m_position;
		toPlayer.y = 0.0f; // Ignore vertical distance
		float distanceToPlayer = glm::length(toPlayer);

		// Only chase if player is within detection range
		if (distanceToPlayer < data.m_detectionRange && distanceToPlayer > 3.0f) // Stop when close
		{
			// Get direction to player
			glm::vec3 direction = data.getDirectionXZtoTarget(playerPosition);

			// Apply movement pattern
			glm::vec3 moveDirection = direction;
			float currentSpeed = data.m_moveSpeed;
			switch (data.m_movementPattern)
			{
			case MovementPattern::DIRECT:
				// Move straight toward player (default behavior)
				break;

			case MovementPattern::ZIGZAG:
			{
				// Calculate perpendicular direction for zigzag
				glm::vec3 perpendicular(-direction.z, 0.0f, direction.x);

				// Oscillate left and right using sine wave
				float zigzagOffset = sin(data.m_movementTimer * data.m_zigzagFrequency) * data.m_zigzagAmplitude;
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
			data.m_position += moveDirection * speed;

			// Update yaw to face the movement direction
			data.m_yaw = glm::degrees(atan2(moveDirection.x, moveDirection.z));
		}

		// Stick to terrain height
		if (m_heightFunc.has_value())
			data.m_position.y = (*m_heightFunc)(data.m_position.x, data.m_position.z);

		// Build transform matrix for this enemy instance
		glm::mat4 transform(1.0f);

		// Translate to world position, adding Y offset so feet touch ground
		glm::vec3 renderPos = data.m_position;
		renderPos.y += data.m_modelYOffset;
		transform = glm::translate(transform, renderPos);

		// Rotate by yaw so model faces forward direction
		transform = glm::rotate(transform, glm::radians(data.m_yaw), glm::vec3(0, 1, 0));

		// Apply hit wobble rotation when damaged
		if (data.m_hitFlashTimer > 0.0f)
		{
			float wobble = sin(data.m_hitFlashTimer * 30.0f) * 0.2f * data.m_hitFlashTimer;
			transform = glm::rotate(transform, wobble, glm::vec3(0, 0, 1));
		}

		// Scale the enemy (with hit pulse effect)
		float currentScale = data.m_modelScale + data.m_hitScaleBoost;
		transform = glm::scale(transform, glm::vec3(currentScale));

		newTransforms.push_back(transform);
	}

	// Upload all transforms to the instanced renderer
	m_instanceRenderer->replaceInstances(newTransforms);
}

void EnemySpawner::spawnNew(glm::vec3 nearPosition, float minDist, float maxDist)
{
	if (m_enemyDataList.size() >= m_maxEnemies)
	{
		DEBUG_PRINT("Enemycount: " << m_maxEnemies << ". Max enemies reached, cannot spawn more.");
		return;
	}

	// Random angle and distance
	float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265f;
	float distance = minDist + static_cast<float>(rand()) / RAND_MAX * (maxDist - minDist);

	// Calculate spawn position
	glm::vec3 spawnPos = nearPosition + glm::vec3(cos(angle), 0.0f, sin(angle)) * distance;

	// Get terrain height if function is set
	if (m_heightFunc.has_value())
	{
		float terrainY = (*m_heightFunc)(spawnPos.x, spawnPos.z);
		spawnPos.y = terrainY;
	}

	// Create new enemy data
	EnemyData newEnemy;
	newEnemy.m_position = spawnPos;

	// Add to list
	m_enemyDataList.push_back(newEnemy);

	DEBUG_PRINT("Spawned new enemy at " << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << ". Total enemies: " << m_enemyDataList.size());
}