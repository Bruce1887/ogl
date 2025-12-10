#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

VertexArray::VertexArray()
{
    GLCALL(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray()
{
    //     DEBUG_PRINT("Deleting VertexArray with ID: " << m_RendererID);
    GLCALL(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::addBuffer(const VertexBuffer *vb, const VertexBufferLayout &layout)
{
    m_VertexCount = vb->getSize() / layout.getStride();
    // std::cout << "[VertexArray] Setting vertex count to " << m_VertexCount << " based on VBO size " << m_VBO_size << " and layout stride " << layout.getStride() << std::endl;

    bind();
    vb->bind();
    const auto &elements = layout.getElements();
    unsigned int offset = 0;

    for (GLuint i = 0; i < elements.size(); i++)
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