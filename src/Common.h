#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Error.h"

#include <iostream>

#define SHADER_DIR "resources/shaders/"
#define TEXTURE_DIR "resources/textures/"

extern GLsizei window_X;
extern GLsizei window_Y;
extern GLFWwindow *window;
extern GLFWmonitor *monitor;

int init(const std::string &windowname);
int exit();