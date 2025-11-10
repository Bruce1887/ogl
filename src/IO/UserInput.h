#pragma once

#include "Common.h"

struct WasdInput
{
    int forward;
    int right;
};

WasdInput getWASDDirection(GLFWwindow *const wdw);

struct MovementInput
{
    WasdInput wasd;
    bool shiftDown;
};

MovementInput getUserMovementInput(GLFWwindow *const wdw);
