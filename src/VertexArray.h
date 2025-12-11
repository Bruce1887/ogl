#pragma once

#include "Common.h"

class VertexBuffer;
class VertexBufferLayout;

class VertexArray
{
    friend class VertexBuffer; // Allow VertexBuffer to access private members 

private:
    // Unique ID assigned by OpenGL
    unsigned int m_RendererID;

    // Number of vertices/elements in the VAO. This is set when calling addBuffer. This is set when calling addBuffer.
    unsigned int m_VertexCount;

public:
    VertexArray();
    ~VertexArray();

    void addBuffer(const VertexBuffer *vb, const VertexBufferLayout& layout);

    unsigned int getID() const { return m_RendererID; }
    unsigned int getCount() const { return m_VertexCount; }
    void bind() const;
    void unbind() const;
};