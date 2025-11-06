#pragma once	

#include <GLFW/glfw3.h>

/**
 * @brief Different hints to opengl about how the data is going to be used.
 * See OpenGL documentation on glBufferData() or glNamedBufferData()
 * 
 * These are just hints and dont enforce any kind of usage, but may help GL optimize stuff.
 * 
 * We will almost exclusively use GL_*_DRAW.
 */
enum class BufferUsage
{
    STREAM_DRAW = GL_STREAM_DRAW,
    STREAM_READ = GL_STREAM_READ,
    STREAM_COPY = GL_STREAM_COPY,
    STATIC_DRAW = GL_STATIC_DRAW,
    STATIC_READ = GL_STATIC_READ,
    STATIC_COPY = GL_STATIC_COPY,
    DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
    DYNAMIC_READ = GL_DYNAMIC_READ,
    DYNAMIC_COPY = GL_DYNAMIC_COPY,
    UNASSIGNED = -1
};