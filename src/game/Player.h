#pragma once

#include <cstdint>
#include <string> 
#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "Entity.h"
#include "Model.h"          // ADDED: Needed for playerModel
#include "ThirdPersonCamera.h" // ADDED: Needed for friend's update/render dependencies

// Forward declarations
class TerrainChunkManager;  // ADDED: Dependency for update
class InputManager;         // ADDED: Dependency for update
struct PhongLightConfig;    // ADDED: Dependency for render

namespace game
{

// Forward-declare Item
struct Item;

// Simple input bundle from client -> server processing. (Keep)
struct PlayerInput
{
    glm::vec3 moveDirection{0.0f}; // normalized movement direction in world/local space
    bool jump{false};              // jump pressed this frame
    bool sprint{false};            // sprint held
    bool attack{false};            // attack action triggered
    float deltaTime{0.0f};         // time elapsed since last input/frame (seconds)
};

// Player: Combined Entity (Game State + World Representation)
class Player : public Entity
{
public:
    using Ptr = std::shared_ptr<Player>;
    using EntityID = Entity::EntityID;

    // --- CONSTRUCTORS (Combined) ---
    // 1. Game State Constructor (Original)
    Player(int clientId = -1, const std::string &name = "Player");

    // 2. World/Render Constructor (Friend's, but slightly modified to be explicit)
    // We initialize the player state AND the 3D model/position here.
    Player(glm::vec3 startPos, const std::string& modelPath, int clientId = -1, const std::string &name = "Player");

    virtual ~Player() = default;

    // --- WORLD / RENDERING FIELDS (From friend's Player.h) ---
    // Note: glm::vec3 position is protected in base Entity, use Entity::position
    float yaw = 0.0f; // Rotation around Y-axis
    Model playerModel;

    // --- GAMEPLAY / UPDATE METHODS (Combined) ---
    
    // RENDER: Implements the rendering logic for the player model in the 3D world.
    void render(glm::mat4 view, glm::mat4 proj, PhongLightConfig* light);
    
    // UPDATE: Implements the friend's position/movement update logic (renamed to UpdateWorldMovement 
    // to separate it from the optional base Entity Update()).
    // Note: We rename it to clearly separate it from the input/network logic.
    void UpdateWorldMovement(float dt, InputManager* input, TerrainChunkManager* terrain);


    // Process input from client/server tick; updates position, stamina and actions. (Original)
    void ProcessInput(const PlayerInput &input); // This implementation should be moved to the CPP file

    // Use currently equipped item. (Original)
    void UseEquippedItem(Entity *target);

    // Optional per-frame hooks (Original, kept for completeness)
    void Update(float deltaTime) override;
    void FixedUpdate(float fixedDelta) override;

    // --- GAME STATE GETTERS/SETTERS (Original) ---
    int getClientId() const { return clientId; } 
    void setClientId(int id) { clientId = id; }
    
    // ... (All other getters/setters for Inventory, Stamina, MoveSpeed)

private:
    int clientId;                                      // network client id
    std::vector<std::shared_ptr<Item>> inventory;      // carried items
    float moveSpeed;                                   // base move speed (units/sec)
    float stamina;                                     // current stamina resource
    bool isSprinting;                                  // whether player is currently sprinting
    size_t equippedIndex;                              // index for equipped item

    // Tunables / defaults 
    static constexpr float defaultMoveSpeed = 5.0f;     
    static constexpr float sprintMultiplier = 1.75f;    
    static constexpr float defaultStamina = 100.0f;     
};

}