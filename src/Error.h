#pragma once

#include <iostream>
#include <filesystem>
#include <stdexcept>

#include <assert.h>

/**
 * Wrap OpenGL calls to clear errors before and check for errors after the call.
 * Usage: GLCALL(glFunction(...));
 */

#define GLCALL(function) \
    GLClearError();      \
    function;            \
    GLLogCall(#function, __FILE__, __LINE__);


#define __FILENAME__ (std::filesystem::path(__FILE__).filename().string())

#ifdef DEBUG
    #define DEBUG_PRINT(msg) \
        do { \
            std::cerr << "\r[" << __FILENAME__ << ":" << __LINE__ << "] " << msg << std::endl; \
        } while (0)
#else
    #define DEBUG_PRINT(msg) do {} while (0)
#endif

void GLClearError();
void GLLogCall(const char *function, const char *file, int line);
