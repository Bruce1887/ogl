#include "VertexArray.h"

VertexArray::VertexArray()
{
    GLCALL(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray()
{
    GLCALL(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::addBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout)
{
    bind();
    vb.bind();
    const auto &elements = layout.getElements();
    unsigned int offset = 0;

    for (size_t i = 0; i < elements.size(); i++)
    {
        const auto &element = elements[i];
        GLCALL(glEnableVertexAttribArray(i));
        GLCALL(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), (const void *)(uintptr_t)offset));
        offset += element.count * VertexBufferElement::getSizeOfType(element.type);
    }
}

void VertexArray::bind() const
{
    GLCALL(glBindVertexArray(m_RendererID));

    RenderingContext *rContext = RenderingContext::Current();
    rContext->m_boundVAO = m_RendererID;    
}

void VertexArray::unbind() const
{
    GLCALL(glBindVertexArray(0));
    RenderingContext *rContext = RenderingContext::Current();
    rContext->m_boundVAO = 0;    
}