#include "Player.h"
#include "Terrain/TerrainChunk.h" // For TerrainChunkManager (must be included or forward declared)
#include "Input/UserInput.h"      // For InputManager (must be included or forward declared)
#include "Model.h"                // For Model (must be included)

#include <glm/gtc/matrix_transform.hpp>
#include <cmath> // For std::sin/cos
#include <iostream>

namespace game
{

// 1. Original Game State Constructor (Basic initialization)
Player::Player(int clientId, const std::string &name)
    : Entity(name, EntityType::PLAYER),
      clientId(clientId),
      moveSpeed(defaultMoveSpeed),
      stamina(defaultStamina),
      isSprinting(false),
      equippedIndex(0)
{
    // playerModel is default constructed (empty)
}

// 2. Combined World/Render Constructor (Initializes model and position)
Player::Player(glm::vec3 startPos, const std::string& modelPath, int clientId, const std::string &name)
    // Initialize base members using the existing constructor
    : Player(clientId, name) 
{
    // Initialize world state/render members
    position = startPos;        // position is protected member of Entity
    playerModel = Model(modelPath);
    // yaw is initialized to 0.0f by default initializer (in header)
}


// --- FRIEND'S RENDER LOGIC (Renamed to fit game::Player::render) ---

void Player::render(glm::mat4 view, glm::mat4 proj, PhongLightConfig* light)
{
    // Build model transform
    glm::mat4 transform(1.0f);

    // translate to world position (using the base Entity::position)
    transform = glm::translate(transform, position);

    // rotate by yaw so model faces forward direction
    transform = glm::rotate(transform, glm::radians(yaw), glm::vec3(0,1,0));

    // optional: scale if your player model is tiny or huge
    transform = glm::scale(transform, glm::vec3(1.0f));

    playerModel.setTransform(transform);
    playerModel.render(view, proj, light);
}


// --- FRIEND'S MOVEMENT LOGIC (Renamed to UpdateWorldMovement) ---

void Player::UpdateWorldMovement(float dt, InputManager* input, TerrainChunkManager* terrain)
{
    // Safety checks
    if (!input || !terrain) return;
    
    // Fetch input
    int forwardMove, rightMove;
    bool shiftDown;
    input->movementInput.fetchMovement(forwardMove, rightMove, shiftDown);

    // Calculate speed
    float speed = moveSpeed * dt; 
    if (shiftDown) speed *= 2.0f;

    // Derive forward vector from yaw
    glm::vec3 forward(
        std::sin(glm::radians(yaw)),
        0,
        std::cos(glm::radians(yaw))
    );

    // Calculate right vector
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));

    // Calculate movement
    glm::vec3 movement(0.0f);
    if (forwardMove != 0) movement += forward * (float)(forwardMove * speed);
    if (rightMove != 0) movement += right * (float)(rightMove * speed);

    position += movement;

    // Terrain collision (skip if no chunks loaded yet)
    if (terrain && !terrain->m_chunks.empty()) {
        float terrainY = terrain->getPreciseHeightAt(position.x, position.z);
        position.y = terrainY;
    } else {
        position.y = 0.0f;  // Ground level
    }

    // --- Turn player left/right ---
    // Note: g_window is a global dependency, typically defined in Common.h
    if (glfwGetKey(g_window, GLFW_KEY_Q) == GLFW_PRESS)
        yaw += 60.0f * dt;

    if (glfwGetKey(g_window, GLFW_KEY_E) == GLFW_PRESS)
        yaw -= 60.0f * dt;
}

// --- ORIGINAL GAME LOGIC (Keep ProcessInput, Update, FixedUpdate) ---

// ProcessInput is still used for the high-level input processing (sprint, attack).
// The ProcessInput logic remains the same (just moving the definition here from the header).
void Player::ProcessInput(const PlayerInput &input)
{
    // ... (Original ProcessInput implementation body)
    isSprinting = input.sprint && (stamina > 0.0f);

    float speed = moveSpeed * (isSprinting ? sprintMultiplier : 1.0f);
    
    // Note: We remove the 'position += input.moveDirection * speed * input.deltaTime' line 
    // from here because the detailed movement is now handled in UpdateWorldMovement
    // to use the mouse/keyboard input manager (friend's method).

    // attack handling
    if (input.attack) {
        UseEquippedItem(nullptr);
    }
    
    // The rest of the stamina/jump/physics handling should occur in a proper game loop or FixedUpdate.
}

void Player::UseEquippedItem(Entity *target)
{
    // ... (Original UseEquippedItem implementation body)
}

void Player::Update(float deltaTime)
{
    // ... (Original Update implementation body)
}

void Player::FixedUpdate(float fixedDelta)
{
    // ... (Original FixedUpdate implementation body)
}


}