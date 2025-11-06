#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "Enums.h"
#include "Error.h"

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
inline const fs::path SHADER_DIR  = fs::path("resources") / "shaders";
inline const fs::path TEXTURE_DIR = fs::path("resources") / "textures";

extern GLsizei window_X;
extern GLsizei window_Y;
extern GLFWwindow *window;
extern GLFWmonitor *monitor;

int oogaboogaInit(const std::string &windowname);
int oogaboogaExit();