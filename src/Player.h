#pragma once
#include <glm/glm.hpp>
#include "Model.h"


class TerrainChunkManager;
class InputManager;

class Player
{
public:
    glm::vec3 position;
    float yaw = 0.0f;

    float moveSpeed = 10.0f;

    Model playerModel;

    Player(glm::vec3 startPos, const std::string& modelPath);
    void update(float dt, InputManager* input, TerrainChunkManager* terrain);

    void render(glm::mat4 view, glm::mat4 proj, PhongLightConfig* light);
};
