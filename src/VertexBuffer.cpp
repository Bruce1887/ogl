#include "VertexBuffer.h"
#include "VertexArray.h"

VertexBuffer::VertexBuffer(const void *cpu_buffer, unsigned int size, VertexArray *associatedVAO, BufferUsage usage)
{
    if (associatedVAO == nullptr)
    {
        std::cerr << "[VertexBuffer] Warning: associatedVAO is nullptr. VertexBuffer should be associated with a VertexArray." << std::endl;
        return;
    }
    associatedVAO->m_VBO_size = size;

    GLCALL(glGenBuffers(1, &m_RendererID));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
    GLCALL(glBufferData(GL_ARRAY_BUFFER, size, cpu_buffer, static_cast<GLenum>(usage)));
}

VertexBuffer::~VertexBuffer()
{
#ifdef DEBUG
    DEBUG_PRINT("Deleting VertexBuffer with ID: " << m_RendererID);
#endif  
    GLCALL(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::bind() const
{
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::unbind() const
{
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
