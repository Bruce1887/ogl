#include "Renderer.h"

void Renderer::clear()
{
    GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::draw(const VertexArray &va, const IndexBuffer &ib, const Shader &shader)
{
    shader.bind();
    va.bind();
    ib.bind();

    GLCALL(glDrawElements(GL_TRIANGLES  , ib.getCount(), GL_UNSIGNED_INT, nullptr));
}

