#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Enums.h"
#include "Error.h"
#include "basic_shapes.h"

#include <iostream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;
inline const fs::path SHADER_DIR  = fs::path("resources") / "shaders";
inline const fs::path TEXTURE_DIR = fs::path("resources") / "textures";

extern GLsizei window_X;
extern GLsizei window_Y;
extern GLFWwindow *window;
extern GLFWmonitor *monitor;

int oogaboogaInit(const std::string &windowname);
int oogaboogaExit();