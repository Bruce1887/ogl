#include "ThirdPersonCamera.h"
#include "Camera.h"
#include "game/Player.h"
#include <GLFW/glfw3.h>
extern GLFWwindow* g_window;  


void ThirdPersonCamera::update(Camera& cam, const Player& player)
{
    float yawRad = glm::radians(player.yaw);

    glm::vec3 forward(
        sin(yawRad),
        0,
        cos(yawRad)
    );

    glm::vec3 camPos = player.getPosition()
                     - forward * distance
                     + glm::vec3(0, height, 0);

    cam.m_Position = camPos;
    cam.m_Target   = player.getPosition() + glm::vec3(0, 1.8f, 0);
}

void ThirdPersonCamera::handlePanning(float dt)
{
    float speed = 5.0f * dt;

    if (glfwGetKey(g_window, GLFW_KEY_R) == GLFW_PRESS)
        height += speed;

    if (glfwGetKey(g_window, GLFW_KEY_F) == GLFW_PRESS)
        height -= speed;

    height = glm::clamp(height, minHeight, maxHeight);
}
