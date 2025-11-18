#pragma once

#include <GLFW/glfw3.h>
#include <assert.h>

/**
 * @brief Base class for different input sources.
 *
 */
class InputSource
{
protected:
    // Indicates if theres input to be handled.
    bool hasUnprocessedInput = false;
};

/**
 * @brief class to hold WASD input state
 *
 * forward: +1 for W, -1 for S, 0 for none.
 * right: +1 for D, -1 for A, 0 for none.
 */
class WasdInput : InputSource
{
public:
    WasdInput() : forward(0), right(0) {}

    int forward;
    int right;
};

/**
 * @brief Get the WASD input state from the given GLFW window.
 *
 * @param wdw Pointer to the GLFW window.
 * @return WasdInput class containing the current WASD input state.
 */
WasdInput getWASDDirection(GLFWwindow *const wdw);

/**
 * @brief class to hold overall movement input state
 *
 * wasd: WASD input state.
 * shiftDown: true if the shift key is currently pressed, false otherwise.
 */
class MovementInput : InputSource
{
public:
    WasdInput wasd;
    bool shiftDown;
};

/**
 * @brief Get the overall movement input state from the given GLFW window.
 *
 * @param wdw Pointer to the GLFW window.
 * @return MovementInput class containing the current movement input state.
 */
MovementInput getUserMovementInput(GLFWwindow *const wdw);

class MouseInput : InputSource
{
private:
    double lastX = 0.0;
    double lastY = 0.0;
    double deltaX;
    double deltaY;
    double scrollX;
    double scrollY;

public:

    bool fetchHasUnprocessedInput() const
    {
        return hasUnprocessedInput;
    };
    void updateDeltas(double xpos, double ypos)
    {
        deltaX = xpos - lastX;
        deltaY = ypos - lastY;
        lastX = xpos;
        lastY = ypos;
        hasUnprocessedInput = true;
    };

    void fetchDeltas(double &outDeltaX, double &outDeltaY)
    {
        // Todo: handle this more smoothly probably
        assert(hasUnprocessedInput && "No unprocessed mouse input available");

        outDeltaX = deltaX;
        outDeltaY = deltaY;
        hasUnprocessedInput = false;
    };
};

/**
 * @brief Get the mouse input state from the given GLFW window.
 *
 * @param wdw Pointer to the GLFW window.
 * @return MouseInput class containing the current mouse input state.
 */
// MouseInput getUserMouseInput(GLFWwindow *const wdw);

class InputManager
{
public:
    InputManager() = default;
    ~InputManager() = default;

    MovementInput movementInput;
    MouseInput mouseInput;
};