#pragma once
#include <glm/glm.hpp>
#include <optional>

class PlayerData;
class Camera;

class ThirdPersonCamera
{
public:
    float m_distance = 12.0f;
    float m_height = 3.0f;
    float m_minHeight = -5.0f;
    float m_maxHeight = 15.0f;

    void update(Camera &cam, const PlayerData &p_data, float dt);

    void setMinHeightFunction(std::function<float(float, float)> func)
    {
        m_minHeightFunc = std::move(func);
    }

private:
    std::optional<std::function<float(float, float)>> m_minHeightFunc;
    void handlePanning(float dt);
};
