#include "IndexBuffer.h"


/**
 * @brief Construct a new Index Buffer:: Index Buffer object
 * 
 * @param data the array of indices. 
 * @param count the amount of indices in data. NOTE: IT IS VERY IMPORTANT THAT THIS NUMBER IS CORRECT AND NOT TOO SMALL (too big might not have an immediate impact but idk)
 * @param usage a hint to the GL implementation of how the data is going to be used (e.g. rendered once and never again, or rendered very many times and changed frequently)
 */
IndexBuffer::IndexBuffer(const unsigned int *data, unsigned int count, BufferUsage usage)
    : m_Count(count)
{
    GLCALL(glGenBuffers(1, &m_RendererID));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
    GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, static_cast<GLenum>(usage)));
}

IndexBuffer::~IndexBuffer()
{
    GLCALL(glDeleteBuffers(1, &m_RendererID));
}

void IndexBuffer::Bind() const
{
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
}

void IndexBuffer::Unbind() const
{
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
