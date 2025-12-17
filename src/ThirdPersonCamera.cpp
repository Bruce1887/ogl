#include "ThirdPersonCamera.h"
#include "Camera.h"
#include "game/Player.h"
#include <GLFW/glfw3.h>

extern GLFWwindow *g_window;

void ThirdPersonCamera::update(Camera &cam, const Player &player, float dt)
{
    float yawRad = glm::radians(player.m_yaw);

    glm::vec3 forward(
        sin(yawRad),
        0,
        cos(yawRad));

    glm::vec3 camPos = player.position - forward * m_distance + glm::vec3(0, m_height, 0);

    handlePanning(dt);
    
    cam.m_Position = camPos;

    if (m_minHeightFunc.has_value())
    {
        float computedMinHeight = (*m_minHeightFunc)(cam.m_Position.x, cam.m_Position.z); 
        DEBUG_PRINT("cam.m_Position.y: " << cam.m_Position.y << "ComputedMinHeight: " << computedMinHeight);

        cam.m_Position.y = glm::max(cam.m_Position.y, computedMinHeight);
    }
    cam.m_Target = player.position + glm::vec3(0, 1.8f, 0);
}

void ThirdPersonCamera::handlePanning(float dt)
{
    float speed = 15.0f * dt;

    if (glfwGetKey(g_window, GLFW_KEY_R) == GLFW_PRESS)
        m_height += speed;

    if (glfwGetKey(g_window, GLFW_KEY_F) == GLFW_PRESS)
        m_height -= speed;

    m_height = glm::clamp(m_height, m_minHeight, m_maxHeight);
}
