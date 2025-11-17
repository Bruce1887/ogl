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

/*
MouseInput getUserMouseInput(GLFWwindow *const wdw)
{
    static double lastX = 0.0;
    static double lastY = 0.0;
    static bool firstCall = true;

    double currentX, currentY;
    glfwGetCursorPos(wdw, &currentX, &currentY);

    MouseInput input;
    if (firstCall)
    {
        input.deltaX = 0.0;
        input.deltaY = 0.0;
        firstCall = false;
    }
    else
    {
        input.deltaX = currentX - lastX;
        input.deltaY = currentY - lastY;
    }

    lastX = currentX;
    lastY = currentY;

    double scrollX = 0.0;
    double scrollY = 0.0;
    // Note: GLFW scroll callback would be better for this, but for simplicity we poll here.
    // This requires more complex state management to track scroll deltas per frame.
    input.scrollX = scrollX;
    input.scrollY = scrollY;

    return input;
}
*/