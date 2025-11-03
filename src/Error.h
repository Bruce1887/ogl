#pragma once

#define GLCALL(x) GLClearError();x;GLLogCall(#x, __FILE__, __LINE__);

void GLClearError();
void GLLogCall(const char* function, const char* file, int line);