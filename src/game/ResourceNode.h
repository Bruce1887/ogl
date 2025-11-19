#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "Entity.h"

namespace game
{

// Type of resource the ResourceNode represents
enum class ResourceType : uint8_t
{
    WOOD,   // tree / wood
    STONE  // stone
    // more?
};

// Forward declaration so we can return/create Item without requiring its full definition here
struct Item;

/*
 ResourceNode
 - Inherits from Entity
 - Represents static, interactable resource nodes (trees, rocks, ore)
 - Server/World is responsible for creating these and calling HandleHit when players/tools hit them.
*/
class ResourceNode : public Entity
{
public:
    using Ptr = std::shared_ptr<ResourceNode>;
    using EntityID = Entity::EntityID;

    // Constructor
    // resourceType: type (WOOD/STONE/IRON)
    // totalYield: number of material units the node yields before being depleted
    // respawnTime: time in seconds until the node respawns (0 = no respawn)
    ResourceNode(ResourceType resourceType = ResourceType::WOOD,
                 int totalYield = 5,
                 float respawnTime = 0.0f,
                 const std::string &name = "ResourceNode")
        : Entity(name,
                 // set an appropriate EntityType for World logic (TREE for wood, ROCK for stone/others)
                 (resourceType == ResourceType::WOOD ? EntityType::TREE : EntityType::ROCK)),
          resourceType(resourceType),
          totalYield(totalYield),
          remainingYield(totalYield),
          timeUntilRespawn(respawnTime),
          respawnTimer(0.0f),
          depleted(false)
    {}

    virtual ~ResourceNode() = default;

    // Returns the resource type
    ResourceType getResourceType() const { return resourceType; }

    // Total capacity (how much it originally yielded)
    int getTotalYield() const { return totalYield; }

    // How much remains to be harvested
    int getRemainingYield() const { return remainingYield; }

    // Respawn time in seconds (0 = no respawn)
    float getRespawnTime() const { return timeUntilRespawn; }
    void setRespawnTime(float t) { timeUntilRespawn = t; }

    // Called when the node is hit / takes damage from player/tools.
    // Overrides Entity::HandleHit — reduces health/remainingYield and triggers DropItems/mark as depleted.
    void HandleHit(float damage) override;

    // Creates and returns Item(s) that drop when the node is destroyed / hit.
    // The server is responsible for inserting these into the World.
    std::vector<std::shared_ptr<Item>> DropItems();

    // Called each tick to handle respawn timer (server-side)
    void Update(float deltaTime) override;

    // Force respawn (can be called by the World)
    void Respawn();

    // Whether the node is currently depleted / inactive
    bool isDepleted() const { return depleted; }

protected:
    ResourceType resourceType;   // which resource this node contains (WOOD/STONE/IRON)
    int totalYield;              // how many units it originally yields
    int remainingYield;          // how much is left to gather
    float timeUntilRespawn;      // time until respawn (seconds); 0 = no respawn
    float respawnTimer;          // internal countdown timer when node is depleted
    bool depleted;               // true if node is empty and waiting to respawn or permanently removed
};

}
