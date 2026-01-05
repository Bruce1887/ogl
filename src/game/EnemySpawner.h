
#pragma once

#include "Enemy.h"
#include "Player.h"
#include "../InstancedRenderer.h"
#include "../AnimatedInstanceRenderer.h"

#include <optional>
#include <glm/glm.hpp>
#include "Audio.h"

/**
 * @brief A class for spawning a single type of enemy and updating them (moving them etc.)
 *
 */
class EnemySpawner
{
public:
	EnemySpawner(const EnemyData &prototypeData, unsigned int maxEnemies = 1, float spawnInterval = 0.5f)
		: m_maxEnemies(maxEnemies), m_spawnInterval(spawnInterval)
	{
		m_animatedInstanceRenderer = std::make_unique<AnimatedInstanceRenderer>();
		m_prototypeEnemyData = prototypeData;
	}
	~EnemySpawner() = default;

	// TODO: change visibility to private later
	std::unique_ptr<AnimatedInstanceRenderer> m_animatedInstanceRenderer;

	void setMinHeightFunction(std::function<float(float, float)> func)
	{
		m_heightFunc = std::move(func);
		DEBUG_PRINT("Set enemy spawner height function.");
	}

	unsigned int enemyCount() const { return static_cast<unsigned int>(m_enemyDataList.size()); }

	void updateAll(float dt, Player &player);

	void renderAll(glm::mat4 view, glm::mat4 proj, PhongLightConfig *light)
	{
		m_animatedInstanceRenderer->render(view, proj, light);
	}

	/**
	 * @brief Spawns a new enemy near the given position
	 *
	 * Creates a new enemy data entry and adds it to the internal list (m_enemyDataList).
	 *
	 * @param nearPosition center position to spawn near
	 * @param minDist minimum distance from the center position
	 * @param maxDist maximum distance from the center position
	 */
	void spawnNew(glm::vec3 nearPosition);

	std::vector<EnemyData> m_enemyDataList;

	void setEntitySounds(const EntitySounds &sounds)
	{
		m_sounds = sounds;
	}

	std::optional<EntitySounds> getEntitySounds() const
	{
		return m_sounds;
	}

private:
	unsigned int m_maxEnemies = 1;
	float m_spawnTimer = 0.0f; // Time since last spawn
	float m_spawnInterval = 0.5f;
	float m_minSpawnDistance = 50.0f;
	float m_maxSpawnDistance = 100.0f;
	float m_despawnThreshold = 320.0f; // If set, enemies beyond this distance from player are despawned

	EnemyData m_prototypeEnemyData; // Prototype enemy data to copy from when spawning new enemies

	// std::unique_ptr<InstancedRenderer> m_instanceRenderer;
	std::optional<std::function<float(float, float)>> m_heightFunc;
	std::optional<EntitySounds> m_sounds = std::nullopt;
};