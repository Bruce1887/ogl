#include "EnemySpawner.h"

void EnemySpawner::updateAll(float dt, Player &player)
{
	glm::vec3 playerPosition = player.m_playerData.m_position;

	// Handle spawning new enemies at intervals
	m_spawnTimer += dt;
	if (m_spawnTimer > m_spawnerConfig.m_spawnInterval)
	{
		m_spawnTimer = 0.0f;
		spawnNew(playerPosition);
	}

	// Prepare map of transforms for instanced rendering
	std::unordered_map<AnimationState, std::vector<glm::mat4>> instanceTransformsByState;
	instanceTransformsByState.reserve(m_enemyDataList.size());

	for (int i = 0; i < m_enemyDataList.size(); i++)
	{
		EnemyData &enemy_data = m_enemyDataList[i];

		// Check if enemy is dead
		if (enemy_data.isDead())
		{
			m_enemyDataList.erase(m_enemyDataList.begin() + i);
			i--;
			continue;
		}

		// Despawn enemies that are too far from the player
		if (glm::length(enemy_data.m_position - playerPosition) > m_spawnerConfig.m_despawnThreshold)
		{
			// DEBUG_PRINT("Despawning enemy at distance " << glm::length(data.m_position - playerPosition));
			m_enemyDataList.erase(m_enemyDataList.begin() + i);
			i--;
			continue;
		}

		// Update timers
		enemy_data.updateTimers(dt);

		// Lock animation state if attacking (attacktimer tracks time since last attack)
		// enemy_data.lockAnimationState(enemy_data.m_attackTimer < enemy_data.m_attackCooldown);

		// Calculate distance to player
		glm::vec3 toPlayer = playerPosition - enemy_data.m_position;
		toPlayer.y = 0.0f; // Ignore vertical distance
		float distanceToPlayer = glm::length(toPlayer);
		glm::vec3 direction_to_player = enemy_data.getDirectionXZtoTarget(playerPosition);

		// Only chase if player is within detection range
		if (distanceToPlayer < enemy_data.m_detectionRange && distanceToPlayer > enemy_data.m_closeRange) // Stop when close
		{
			enemy_data.setAnimationState(AnimationState::WALKING);
			// Get direction to player

			// Apply movement pattern
			glm::vec3 moveDirection = direction_to_player;
			float currentSpeed = enemy_data.m_moveSpeed;
			switch (enemy_data.m_movementPattern)
			{
			case MovementPattern::DIRECT:
				// Move straight toward player (default behavior)
				break;
			case MovementPattern::ZIGZAG:
			{
				// Calculate perpendicular direction for zigzag
				glm::vec3 perpendicular(-direction_to_player.z, 0.0f, direction_to_player.x);
				// Oscillate left and right using sine wave
				float zigzagOffset = sin(enemy_data.m_movementTimer * enemy_data.m_zigzagFrequency) * enemy_data.m_zigzagAmplitude;
				moveDirection = glm::normalize(direction_to_player + perpendicular * zigzagOffset * 0.3f);
				break;
			}
			case MovementPattern::CAUTIOUS:
				// Move slower overall
				currentSpeed *= 0.7f;
				break;
			}

			// Move toward player with pattern applied
			float speed = currentSpeed * dt;
			enemy_data.m_position += moveDirection * speed;

			// Update yaw to face the movement direction
			enemy_data.m_yaw = glm::degrees(atan2(moveDirection.x, moveDirection.z));
		}
		else if (distanceToPlayer <= enemy_data.m_closeRange)
		{
			enemy_data.setAnimationState(AnimationState::ATTACK);

			// The enemy is in range to attack the player
			float damage = enemy_data.tryAttack(dt);
			if (damage > 0.0f)
			{
				player.m_playerData.take_damage(damage);
				// Play attack sound if available
				if (m_sounds.has_value())
				{
					SoundPlayer::getInstance().PlaySFX((*m_sounds).m_attackSound);
				}
			}
		}

		// Stick to terrain height
		if (m_heightFunc.has_value())
			enemy_data.m_position.y = (*m_heightFunc)(enemy_data.m_position.x, enemy_data.m_position.z);

		// Build transform matrix for this enemy instance
		glm::mat4 transform(1.0f);

		// Translate to world position, adding Y offset so feet touch ground
		glm::vec3 renderPos = enemy_data.m_position;
		renderPos.y += enemy_data.m_modelYOffset;
		transform = glm::translate(transform, renderPos);

		// Rotate by yaw so model faces forward direction
		transform = glm::rotate(transform, glm::radians(enemy_data.m_yaw), glm::vec3(0, 1, 0));

		// Apply hit wobble rotation when damaged
		if (enemy_data.m_hitFlashTimer > 0.0f)
		{
			float wobble = sin(enemy_data.m_hitFlashTimer * 30.0f) * 0.2f * enemy_data.m_hitFlashTimer;
			transform = glm::rotate(transform, wobble, glm::vec3(0, 0, 1));
		}

		// Scale the enemy (with hit pulse effect)
		float currentScale = enemy_data.m_modelScale + enemy_data.m_hitScaleBoost;
		transform = glm::scale(transform, glm::vec3(currentScale));

		instanceTransformsByState[enemy_data.getAnimationState()].push_back(transform);
	}

	// Upload all transforms to the AnimatedInstancerenderer
	m_animatedInstanceRenderer->updateInstances(instanceTransformsByState, dt);
	// m_instanceRenderer->replaceInstances(new_instanceData);
}

void EnemySpawner::spawnNew(glm::vec3 nearPosition)
{
	if (m_enemyDataList.size() >= m_spawnerConfig.m_maxEnemies || !m_active)
	{
		// DEBUG_PRINT("Enemycount: " << m_maxEnemies << ". Max enemies reached, cannot spawn more.");
		return;
	}

	// Random angle and distance
	float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265f;
	float distance = m_spawnerConfig.m_minSpawnDistance + static_cast<float>(rand()) / RAND_MAX * (m_spawnerConfig.m_maxSpawnDistance - m_spawnerConfig.m_minSpawnDistance);

	// Calculate spawn position
	glm::vec3 spawnPos = nearPosition + glm::vec3(cos(angle), 0.0f, sin(angle)) * distance;

	// Get terrain height if function is set
	if (m_heightFunc.has_value())
	{
		float terrainY = (*m_heightFunc)(spawnPos.x, spawnPos.z);
		spawnPos.y = terrainY;
	}

	// Create new enemy data
	EnemyData newEnemy = m_prototypeEnemyData;
	newEnemy.m_position = spawnPos;

	// Add to list
	m_enemyDataList.push_back(newEnemy);

	// DEBUG_PRINT("Spawned new enemy at " << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << ". Total enemies: " << m_enemyDataList.size());
}