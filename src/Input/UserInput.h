#pragma once

#include <GLFW/glfw3.h>
#include <assert.h>

#include <iostream>

/**
 * @brief Base class for different input sources.
 *
 */
class InputSource
{
private:
    // Indicates if theres input to be handled.
    bool hasUnprocessedInput = false;

protected:
    void markUpdated() { hasUnprocessedInput = true; }
    void clearUpdated() { hasUnprocessedInput = false; }
    bool hasInput() const { return hasUnprocessedInput; }

    // would be nice to have these as abstract methods, but the signature varies greatly between different input sources. maybe find a workaround later.
    // public:
    //     virtual void update() = 0;
    //     virtual bool fetchUpdates() = 0;
};

/**
 * @brief class to hold all keyboard-related movement input state
 *
 * wasd: WASD input state.
 * shiftDown: true if the shift key is currently pressed, false otherwise.
 */
class MovementInput : public InputSource
{
    /**
     * @brief struct to hold WASD input state
     */
    struct WasdInput
    {
        bool w_down;
        bool a_down;
        bool s_down;
        bool d_down;
    };
    WasdInput wasd;
    bool shiftDown;

public:
    // Call this method to update movement input state, and set hasUnprocessedInput to true. Intended to be called from a key callback.
    void updateMovement(int key, int action, int /* mods */);

    void fetchMovement(int &outForward, int &outRight, bool &outShiftDown);
};

class MouseMoveInput : public InputSource
{
private:
    double lastX = 0.0;
    double lastY = 0.0;
    double deltaX;
    double deltaY;

public:
    // Call this method to update deltas, and set hasUnprocessedInput to true. Intended to be called from a cursor position callback.
    void updateDeltas(double xpos, double ypos);

    // Call this method when fetching the deltas to reset the unprocessed input flag
    bool fetchDeltas(double &outDeltaX, double &outDeltaY);
};

class MouseScrollInput : public InputSource
{
private:
    double scrollX = 0.0;
    double scrollY = 0.0;

public:
    // Call this method to update scroll offsets, and set hasUnprocessedInput to true. Intended to be called from a scroll callback.
    void updateScroll(double xoffset, double yoffset);

    bool fetchScroll(double &outScrollX, double &outScrollY);
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
    MouseMoveInput mouseMoveInput;
    MouseScrollInput scrollInput;
};