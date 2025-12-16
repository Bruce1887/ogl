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
    double newDeltaX = xpos - lastX;
    double newDeltaY = ypos - lastY;
    
    // ACCUMULATE deltas instead of replacing them
    deltaX += newDeltaX;
    deltaY += newDeltaY;
    
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
    deltaX = 0.0;
    deltaY = 0.0;
    clearUpdated();
    return true;
}

// MouseScrollInput implementations
void MouseScrollInput::updateScroll(double xoffset, double yoffset)
{
    scrollX += xoffset;
    scrollY += yoffset;
    markUpdated();
}

bool MouseScrollInput::fetchScroll(double &outScrollX, double &outScrollY)
{
    if (!hasInput())
        return false;

    outScrollX = scrollX;
    outScrollY = scrollY;
    scrollX = 0.0;
    scrollY = 0.0;
    clearUpdated();
    return true;
}

// AttackInput implementations
void AttackInput::triggerAttack()
{
    attackPressed = true;
    markUpdated();
}

bool AttackInput::fetchAttack()
{
    if (!attackPressed)
        return false;
    
    attackPressed = false;
    clearUpdated();
    return true;
}