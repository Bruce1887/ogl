#pragma once

/**
 * Wrap OpenGL calls to clear errors before and check for errors after the call.
 * Usage: GLCALL(glFunction(...));
 */
#define GLCALL(x) GLClearError();x;GLLogCall(#x, __FILE__, __LINE__);

void GLClearError();
void GLLogCall(const char* function, const char* file, int line);