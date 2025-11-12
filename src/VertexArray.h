#pragma once

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "Common.h"

class VertexArray
{
private:
    unsigned int m_RendererID; 

public:
    VertexArray();
    ~VertexArray();

    void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);


    unsigned int getID() const { return m_RendererID; }
    void bind() const;
    void unbind() const;
};