#pragma once

#include <cstdint>
#include <string> 
#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "Entity.h"

namespace game
{

// Forward-declare Item so inventory can store pointers without depending on full definition.
struct Item;

// Simple input bundle from client -> server processing.
// Contains movement direction, actions and a per-frame delta time.
struct PlayerInput
{
    glm::vec3 moveDirection{0.0f}; // normalized movement direction in world/local space
    bool jump{false};              // jump pressed this frame
    bool sprint{false};            // sprint held
    bool attack{false};            // attack action triggered
    float deltaTime{0.0f};         // time elapsed since last input/frame (seconds)
};

// Player: a server-authoritative, playable entity. Inherits common fields from Entity.
class Player : public Entity
{
public:
    using Ptr = std::shared_ptr<Player>;
    using EntityID = Entity::EntityID;

    // Construct a player with optional name and client id; sets EntityType::PLAYER.
    Player(int clientId = -1, const std::string &name = "Player")
        : Entity(name, EntityType::PLAYER),
        clientId(clientId),
        moveSpeed(defaultMoveSpeed),
        stamina(defaultStamina),
        isSprinting(false),
        equippedIndex(0)
    {}

    virtual ~Player() = default;

    // Network / identification
    int getClientId() const { return clientId; }          // network client ID associated with this player
    void setClientId(int id) { clientId = id; }

    // Inventory access (shared_ptr<Item> so header doesn't require Item definition)
    const std::vector<std::shared_ptr<Item>>& getInventory() const { return inventory; }
    void addToInventory(const std::shared_ptr<Item>& item) { inventory.push_back(item); }

    // Movement / gameplay fields
    float getMoveSpeed() const { return moveSpeed; }
    void setMoveSpeed(float s) { moveSpeed = s; }

    float getStamina() const { return stamina; }
    void setStamina(float s) { stamina = s; }

    bool getIsSprinting() const { return isSprinting; }
    void setIsSprinting(bool v) { isSprinting = v; }

    // Equipped item index (index into inventory). -1 = none.
    int getEquippedIndex() const { return static_cast<int>(equippedIndex); }
    void setEquippedIndex(size_t idx) { equippedIndex = idx; }

    // Process input from client/server tick; updates position, stamina and actions.
    // Implemented here with minimal logic; extend for authoritative movement, collision, etc.
    void ProcessInput(const PlayerInput &input)
    {
        // sprint state affects speed and stamina drain
        isSprinting = input.sprint && (stamina > 0.0f);

        float speed = moveSpeed * (isSprinting ? sprintMultiplier : 1.0f);
        // Integrate simple position: position is protected in base Entity, accessible here.
        position += input.moveDirection * speed * input.deltaTime;

        // simple stamina handling
        /*if (isSprinting) {
            stamina -= staminaDrainPerSecond * input.deltaTime;
            if (stamina < 0.0f) stamina = 0.0f;
        } else {
            // regenerate stamina when not sprinting
            stamina += staminaRegenPerSecond * input.deltaTime;
            if (stamina > defaultStamina) stamina = defaultStamina;
        }*/

        // attack handling (server should validate & apply damage)
        if (input.attack) {
            // placeholder: call UseEquippedItem(nullptr) for self-target or close-range hit
            UseEquippedItem(nullptr);
        }

        // jumping, physics, collisions, and authoritative reconciliation are not handled here.
    }

    // Use currently equipped item on a target entity (target may be nullptr for world interaction).
    // Server-side logic should resolve effects, apply damage, spawn events, etc.
    void UseEquippedItem(Entity *target)
    {
        if (equippedIndex >= inventory.size()) return; // nothing equipped

        auto &item = inventory[equippedIndex];
        if (!item) return;

        // Example placeholder behaviour: Items should expose a Use(...) method on real implementation.
        // Here we only demonstrate intent; actual item effects are implemented in Item.
        // item->Use(this, target);
    }

    // Optional per-frame hooks (override base Entity behaviour)
    void Update(float deltaTime) override
    {
        (void)deltaTime; // server-side AI, timers or status effects could be processed here
    }

    void FixedUpdate(float fixedDelta) override
    {
        (void)fixedDelta; // deterministic tick updates (physics, network reconciliation)
    }

private:
    int clientId;                                      // network client id for this player (server-side)
    std::vector<std::shared_ptr<Item>> inventory;      // player's carried items (pointers so header can forward-declare Item)
    float moveSpeed;                                   // base move speed (units/sec)
    float stamina = 100.0f;                                     // current stamina resource
    float health = 100.0f;                                      // current health points
    float maxHealth = 100.0f;                                   // maximum health points
    bool isSprinting;                                  // whether player is currently sprinting
    size_t equippedIndex;                              // index into inventory for the equipped item

    // Tunables / defaults (can be static config or loaded values)
    static constexpr float defaultMoveSpeed = 5.0f;           // default walking speed
    static constexpr float sprintMultiplier = 1.75f;         // multiplier when sprinting
    static constexpr float defaultStamina = 100.0f;          // maximum stamina
    //static constexpr float staminaDrainPerSecond = 20.0f;    // stamina drained per second while sprinting
    //static constexpr float staminaRegenPerSecond  = 10.0f;   // stamina regenerated per second while idle/not sprinting
};

} 