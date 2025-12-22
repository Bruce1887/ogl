#pragma once

#include "InputSource.h"
#include "KeyboardInput.h"

#include <GLFW/glfw3.h>
#include <assert.h>
#include <iostream>

class MouseMoveInput : public InputSource
{
private:
    double lastX = 0.0;
    double lastY = 0.0;
    double deltaX;
    double deltaY;

public:
    // Call this method to update deltas, and set hasUnprocessedInput to true. Intended to be called from a cursor position callback.
    void update(const InputUpdate &updateData) override;

    // Call this method when fetching the deltas to reset the unprocessed input flag
    bool fetchDeltas(double &outDeltaX, double &outDeltaY);

    void fetchLastPosition(double &outLastX, double &outLastY){
        outLastX = lastX;
        outLastY = lastY;
    }
};

class MouseScrollInput : public InputSource
{
private:
    double scrollX = 0.0;
    double scrollY = 0.0;

public:
    // Call this method to update scroll offsets, and set hasUnprocessedInput to true. Intended to be called from a scroll callback.
    void update(const InputUpdate &updateData) override;

    bool fetchScroll(double &outScrollX, double &outScrollY);
};

class MouseButtonInput : public InputSource
{
private:
    bool leftButtonDown = false;
    bool rightButtonDown = false;
public:
    void update(const InputUpdate &updateData) override; 
    bool fetchButtons(bool &outLeftButtonDown, bool &outRightButtonDown);
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

    KeyboardInput keyboardInput;
    MouseMoveInput mouseMoveInput;
    MouseScrollInput scrollInput;
    MouseButtonInput mouseButtonInput;
};