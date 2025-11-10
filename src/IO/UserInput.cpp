#include "UserInput.h"

WasdInput getWASDDirection(GLFWwindow *const wdw)
{

    WasdInput wasd = {.forward = 0, .right = 0};
    if (glfwGetKey(wdw, GLFW_KEY_W) == GLFW_PRESS)
        wasd.forward += 1;
    if (glfwGetKey(wdw, GLFW_KEY_S) == GLFW_PRESS)
        wasd.forward -= 1;
    if (glfwGetKey(wdw, GLFW_KEY_A) == GLFW_PRESS)
        wasd.right -= 1;
    if (glfwGetKey(wdw, GLFW_KEY_D) == GLFW_PRESS)
        wasd.right += 1;

    return wasd;
}

MovementInput getUserMovementInput(GLFWwindow *const wdw)
{
    MovementInput input;
    input.wasd = getWASDDirection(wdw);
    input.shiftDown = (glfwGetKey(wdw, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
    return input;
}
