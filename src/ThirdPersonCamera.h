#pragma once
#include <glm/glm.hpp>

namespace game { class Player; }
class Camera;

class ThirdPersonCamera
{
public:
    float distance = 8.0f;
    float height   = 3.0f;
    float minHeight = 1.0f;
    float maxHeight = 10.0f;

    void handlePanning(float dt);
    void update(Camera& cam, const game::Player& player); // la till game::Player  för forward declaration
};

