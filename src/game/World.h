#pragma once

#include <unordered_map>
#include <cstdint>
#include <glm/glm.hpp>

#include "Entity.h"
#include "Player.h"
#include "ResourceNode.h"
#include "TerrainManager.h"
#include "Terrainchunk.h"

// Include your Terrain/Chunk headers here

namespace game
{

class World {
public:
    // Stores all active dynamic entities by their unique ID
    std::unordered_map<Entity::EntityID, Entity*> m_Entities;
    Entity::EntityID m_NextEntityID = 1;

    // Players keyed by EntityID (or client ID if you prefer)
    std::unordered_map<Entity::EntityID, Player*> m_Players;

    void Initialize();
    void Update(float deltaTime);
    
    // Spawning/Access
    Player* SpawnPlayer(const glm::vec3 &spawnPos, Entity::EntityID clientID);
    void RemoveEntity(Entity::EntityID entityID);
    
    // Terrain Integration (Placeholder function)
    bool IsSolidBlock(const glm::vec3 &position) const; 

private:
    // Terrain and Chunk Manager instance (assuming you have one)
    // TerrainManager* m_Terrain; 
    
    // Helper for night time enemy spawning
    void CheckNightEvents();
};

}