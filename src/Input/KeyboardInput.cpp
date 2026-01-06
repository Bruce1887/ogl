#include "KeyboardInput.h"

// MovementInput implementations
void MovementInput::update(const InputUpdate &updateData)
{
#ifdef DEBUG
    assert(std::holds_alternative<KeyboardUpdate>(updateData) && "MovementInput received wrong InputUpdate variant");
#endif
    const KeyboardUpdate &movementUpdate = std::get<KeyboardUpdate>(updateData);
    int key = movementUpdate.key;
    int action = movementUpdate.action;

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

KeyState &KeyboardInput::getKeyState(int keyCode)
{
    for (KeyState &ks : keyStates)
    {
        if (ks.key() == keyCode)
        {
            return ks;
        }
    }

    assert(false && "KeyState for given keyCode not found!");

    static KeyState dummy = KeyState(-1);
    return dummy;
}