#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define SHADER_DIR "resources/shaders/"
#define TEXTURE_DIR "resources/textures/"

extern GLsizei window_X;
extern GLsizei window_Y;
extern GLFWwindow *window;
extern GLFWmonitor *monitor;

int init();
int exit();