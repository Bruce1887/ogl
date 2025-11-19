#include "UserInput.h"

// MovementInput implementations
void MovementInput::updateMovement(int key, int action, int /* mods */)
{
    bool isPressed = action != GLFW_RELEASE;
    switch (key)
    {
    case GLFW_KEY_W:
        wasd.w_down = isPressed;
        break;
    case GLFW_KEY_A:
        wasd.a_down = isPressed;
        break;
    case GLFW_KEY_S:
        wasd.s_down = isPressed;
        break;
    case GLFW_KEY_D:
        wasd.d_down = isPressed;
        break;
    case GLFW_KEY_LEFT_SHIFT:
        shiftDown = isPressed;
        break;
    default:
        break;
    }
    markUpdated();
}

void MovementInput::fetchMovement(int &outForward, int &outRight, bool &outShiftDown)
{
    outForward = (wasd.w_down ? 1 : 0) + (wasd.s_down ? -1 : 0);
    outRight = (wasd.d_down ? 1 : 0) + (wasd.a_down ? -1 : 0);
    outShiftDown = shiftDown;
    clearUpdated(); 
}

// MouseMoveInput implementations
void MouseMoveInput::updateDeltas(double xpos, double ypos)
{
    deltaX = xpos - lastX;
    deltaY = ypos - lastY;
    lastX = xpos;
    lastY = ypos;
    markUpdated();
}

bool MouseMoveInput::fetchDeltas(double &outDeltaX, double &outDeltaY)
{
    if (!hasInput())
        return false;

    outDeltaX = deltaX;
    outDeltaY = deltaY;
    clearUpdated();
    return true;
}

// MouseScrollInput implementations
void MouseScrollInput::updateScroll(double xoffset, double yoffset)
{
    scrollX = xoffset;
    scrollY = yoffset;
    markUpdated();
}

bool MouseScrollInput::fetchScroll(double &outScrollX, double &outScrollY)
{
    if (!hasInput())
        return false;

    std::cout << "Fetching scroll: " << scrollX << ", " << scrollY << std::endl;
    outScrollX = scrollX;
    outScrollY = scrollY;
    clearUpdated();
    return true;
}