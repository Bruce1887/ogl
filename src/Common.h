#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Enums.h"
#include "Error.h"
#include "basic_shapes.h"
#include "RenderingContext.h"
#include "Input/UserInput.h"

#include <iostream>
#include <filesystem>
#include <vector>
#include <memory>

namespace fs = std::filesystem;
inline const fs::path VERTEX_SHADER_DIR = fs::path("resources") / "shaders" / "vertex";
inline const fs::path FRAGMENT_SHADER_DIR = fs::path("resources") / "shaders" / "fragment";

inline const fs::path TEXTURE_DIR = fs::path("resources") / "textures";

inline const fs::path MODELS_DIR = fs::path("resources") / "models";

inline const fs::path FONTS_DIR = fs::path("resources") / "fonts";
#define MENU_FONT_SIZE 48

// The amount of texture units we expect to have available
const int REQUIRED_NUM_TEXTURE_UNITS = 32;

// Global variables
extern RenderingContext *rContext;
extern GLsizei WINDOW_X;
extern GLsizei WINDOW_Y;
extern GLFWwindow *g_window;
extern GLFWmonitor *g_monitor;
extern InputManager *g_InputManager;

/**
 * Sets up various stuff which is mandatory to get OpenGL running with this framework, and some nice to have stuff as well.
 *
 * Sets up a GLFW window with OpenGL context, initializes GLAD, creates a RenderingContext instance and makes it current,
 *
 * @param windowname Name of the created window.
 *
 */
int oogaboogaInit(const std::string &windowname);

/**
 * Clean up whatever allocations were made in oogaboogaInit(), and call glfwTerminate().
 */
int oogaboogaExit();