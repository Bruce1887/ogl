#pragma once

#include "Common.h"

/**
 * @brief Struct to hold WASD input state
 * 
 * forward: +1 for W, -1 for S, 0 for none.
 * right: +1 for D, -1 for A, 0 for none.
 */
struct WasdInput
{
    int forward;
    int right;
};

/**
 * @brief Get the WASD input state from the given GLFW window.
 * 
 * @param wdw Pointer to the GLFW window.
 * @return WasdInput Struct containing the current WASD input state.
 */
WasdInput getWASDDirection(GLFWwindow *const wdw);

/** 
 * @brief Struct to hold overall movement input state
 * 
 * wasd: WASD input state.
 * shiftDown: true if the shift key is currently pressed, false otherwise.
 */
struct MovementInput
{
    WasdInput wasd;
    bool shiftDown;
};

/**
 * @brief Get the overall movement input state from the given GLFW window.
 * 
 * @param wdw Pointer to the GLFW window.
 * @return MovementInput Struct containing the current movement input state.
 */
MovementInput getUserMovementInput(GLFWwindow *const wdw);
