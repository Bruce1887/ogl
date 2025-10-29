#pragma once

#include <glad/glad.h>

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h" 


#define GLCALL(x) GLClearError();x;GLLogCall(#x, __FILE__, __LINE__);

void GLClearError();
void GLLogCall(const char* function, const char* file, int line);

class Renderer
{
public:
    void Clear() const;
    void Draw(const VertexArray &va, const IndexBuffer &ib, const Shader &shader) const;

};
