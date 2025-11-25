#pragma once

#include <iostream>

/**
 * Wrap OpenGL calls to clear errors before and check for errors after the call.
 * Usage: GLCALL(glFunction(...));
 */

#define GLCALL(function) \
    GLClearError();      \
    function;            \
    GLLogCall(#function, __FILE__, __LINE__);

// Print macro (prepends file and line number)
#define DEBUG_PRINT(msg)                                                     \
    do                                                                       \
    {                                                                        \
        std::cerr << __FILE__ << ":" << __LINE__ << " " << msg << std::endl; \
    } while (0)

void GLClearError();
void GLLogCall(const char *function, const char *file, int line);
