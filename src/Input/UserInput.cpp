#include "UserInput.h"

// MouseMoveInput implementations
void MouseMoveInput::update(const InputUpdate &updateData)
{
#ifdef DEBUG
    assert(std::holds_alternative<MousePosUpdate>(updateData) && "MouseMoveInput received wrong InputUpdate variant");
#endif
    const MousePosUpdate &mousePosUpdate = std::get<MousePosUpdate>(updateData);
    double xpos = mousePosUpdate.x;
    double ypos = mousePosUpdate.y;

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
void MouseScrollInput::update(const InputUpdate &updateData)
{
#ifdef DEBUG
    assert(std::holds_alternative<ScrollUpdate>(updateData) && "MouseScrollInput received wrong InputUpdate variant");
#endif
    const ScrollUpdate &scrollUpdate = std::get<ScrollUpdate>(updateData);
    double xoffset = scrollUpdate.xoffset;
    double yoffset = scrollUpdate.yoffset;

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

// MouseButtonInput implementations
void MouseButtonInput::update(const InputUpdate &updateData)
{
#ifdef DEBUG
    assert(std::holds_alternative<ButtonUpdate>(updateData) && "MouseButtonInput received wrong InputUpdate variant");
#endif
    const ButtonUpdate &buttonUpdate = std::get<ButtonUpdate>(updateData);
    int button = buttonUpdate.button;
    int action = buttonUpdate.action;

    bool isPressed = action != GLFW_RELEASE;
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        leftButtonDown = isPressed;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        rightButtonDown = isPressed;
        break;
    default:
        break;
    }
    markUpdated();
}

bool MouseButtonInput::fetchButtons(bool &outLeftButtonDown, bool &outRightButtonDown)
{
    if (!hasInput())
        return false;
    outLeftButtonDown = leftButtonDown;
    outRightButtonDown = rightButtonDown;
    clearUpdated();
    return true;
}