#pragma once

#include <cstdint>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>     // glm::quat (quaternions for rotation)
#include <memory>

namespace game 
{

// EntityType: used to categorize entities (server/world logic uses this)
enum class EntityType : uint8_t
{
    UNKNOWN = 0,                       // default / unspecified type
    PLAYER,                            // player-controlled entity
    MOB,                               // mobile AI enemy or creature
    TREE,                              // static environment object
    ROCK                               // another static object
    // add more types as needed
};

// The Entity class: lightweight data + virtual hooks for game logic
class Entity
{
public:
    using EntityID = uint32_t;         // alias for the unique ID type

    // Constructor: assigns a unique id
    Entity(const std::string &name = "Entity",
           EntityType type = EntityType::UNKNOWN)
        : id(nextId()), name(name), type(type) {}

    virtual ~Entity() = default;       // virtual destructor for proper cleanup in derived classes

    // Core data accessors (server-authoritative fields)

    EntityID getId() const { return id; }                 // return the unique ID
    const std::string &getName() const { return name; }   // return the entity's name
    void setName(const std::string &n) { name = n; }      // set the entity's name

    // Position getter/setter (3D world position)
    glm::vec3 getPosition() const { return position; }
    void setPosition(const glm::vec3 &p) { position = p; }

    // Rotation exposed as Euler angles (degrees) for convenience / networking
    glm::vec3 getRotationEuler() const { return rotationEuler; } // get Euler angles (deg)
    // set Euler angles and update internal quaternion representation
    void setRotationEuler(const glm::vec3 &r) { rotationEuler = r; rotation = glm::quat(glm::radians(r)); }

    // Quaternion rotation accessors 
    glm::quat getRotationQuat() const { return rotation; }
    // set quaternion and update Euler copy (degrees) for external use
    void setRotationQuat(const glm::quat &q) { rotation = q; rotationEuler = glm::degrees(glm::eulerAngles(q)); }

    // Health accessors (gameplay state)
    float getHealth() const { return currentHealth; }
    void setHealth(float h) { currentHealth = h; }

    // Active flag (whether world should process this entity)
    bool isActive() const { return active; }
    void setActive(bool a) { active = a; }

    // Type accessors
    EntityType getType() const { return type; }
    void setType(EntityType t) { type = t; }

    // Game loop hooks to override in derived classes:
    // Update called every frame / tick with variable deltaTime
    virtual void Update(float deltaTime) { (void)deltaTime; }        // default: no-op
    // FixedUpdate for deterministic updates (physics / server tick)
    virtual void FixedUpdate(float fixedDelta) { (void)fixedDelta; } // default: no-op
    // HandleHit reduces health by damage (can be overridden for custom behavior)
    virtual void HandleHit(float damage) { currentHealth -= damage; if (currentHealth < 0.0f) currentHealth = 0.0f; }

protected:
    // Unique ID (used by network & world) - const so it can't change after construction
    const EntityID id;
    std::string name;                  // human-readable name or identifier

    // Transform
    glm::vec3 position{0.0f};          // world-space position (x,y,z)
    glm::vec3 rotationEuler{0.0f};     // Euler angles in degrees (for serialization / editor)
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f}; // quaternion (w,x,y,z) default identity rotation

    // Gameplay / runtime state
    float currentHealth{100.0f};       // current health points
    bool active{true};                 // whether entity is active (processed by World)

    EntityType type{EntityType::UNKNOWN}; // semantic type/category of the entity

private:
    // Generates a monotonically increasing unique ID for each Entity instance
    static EntityID nextId()
    {
        static EntityID s_next = 1; // start IDs at 1
        return s_next++;
    }
};

using EntityPtr = std::shared_ptr<Entity>; // convenience alias for shared pointers to Entities

}