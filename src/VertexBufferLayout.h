#pragma once

#include <vector>
#include <glad/glad.h>
#include <iostream>

struct VertexBufferElement
{
    unsigned int type;
    unsigned int count;
    unsigned char normalized;

    static unsigned int GetSizeOfType(unsigned int type)
    {
        switch (type)
        {
        case GL_FLOAT:
            return 4;
        case GL_UNSIGNED_INT:
            return 4;
        case GL_UNSIGNED_BYTE:
            return 1;
        default:
            std::cerr << "[VertexBufferElement] Unknown type: " << type << std::endl;
            return 0;
        }
    }
};

class VertexBufferLayout
{
private:
    std::vector<VertexBufferElement> m_Elements;
    unsigned int m_stride;

public:
    VertexBufferLayout()
        : m_stride(0) {}
    // ~VertexBufferLayout(); // dont need destructor as we dont have any dynamic memory allocation

    template <typename T>
inline void Push(unsigned int count)
{
    static_assert(sizeof(T) == 0, "Push<T> not implemented for this type. Use float, unsigned int, or unsigned char.");  
}
    inline const std::vector<VertexBufferElement> &GetElements() const { return m_Elements; }

    inline unsigned int GetStride() const { return m_stride; }
};

template <>
inline void VertexBufferLayout::Push<float>(unsigned int count)
{
    m_Elements.push_back({GL_FLOAT, count, GL_FALSE});
    m_stride += VertexBufferElement::GetSizeOfType(GL_FLOAT) * count;
}

template <>
inline void VertexBufferLayout::Push<unsigned int>(unsigned int count)
{
    m_Elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
    m_stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT) * count;
}

template <>
inline void VertexBufferLayout::Push<unsigned char>(unsigned int count)
{
    m_Elements.push_back({GL_UNSIGNED_BYTE, count, GL_TRUE});
    m_stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE) * count;
}