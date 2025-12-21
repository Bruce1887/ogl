#pragma once

#include "Enemy.h"
#include "../InstancedRenderer.h"

#include <optional>
#include <glm/glm.hpp>

/**
 * @brief A class for spawning a single type of enemy and updating them (moving them etc.)
 *
 */
class EnemySpawner
{
public:
	EnemySpawner(const std::filesystem::path modelPath)
	{
		m_instanceRenderer = std::make_unique<InstancedRenderer>();
		m_enemyModel = std::make_unique<Model>(modelPath);
		m_instanceRenderer->init(m_enemyModel.get());
	}
	~EnemySpawner() = default;

	void setMinHeightFunction(std::function<float(float, float)> func)
	{
		m_heightFunc = std::move(func);
		DEBUG_PRINT("Set enemy spawner height function.");
	}

	unsigned int enemyCount() const { return static_cast<unsigned int>(m_enemyDataList.size()); }

	void updateAll(float dt, const glm::vec3 &playerPosition);

	void renderAll(glm::mat4 view, glm::mat4 proj, PhongLightConfig *light)
	{
		m_instanceRenderer.get()->render(view, proj, light);
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
	void spawnNew(glm::vec3 nearPosition, float minDist, float maxDist);

	std::unique_ptr<Model> m_enemyModel;

private:
	unsigned int m_maxEnemies = 100;	
	
	float m_spawnTimer = 0.0f; // Time since last spawn
	float m_spawnInterval = 1.0f; 

	std::vector<EnemyData> m_enemyDataList;
	std::unique_ptr<InstancedRenderer> m_instanceRenderer;
	std::optional<std::function<float(float, float)>> m_heightFunc;
};