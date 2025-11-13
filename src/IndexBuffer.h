#pragma once

#include "Common.h"

class IndexBuffer
{
private:
    GLuint m_RendererID; // Unique ID for the buffer
    unsigned int m_Count; // Number of indices
public:
    
    IndexBuffer(const unsigned int *data, unsigned int count, BufferUsage usage = BufferUsage::STATIC_DRAW);
    
    template<typename T>
    IndexBuffer(const std::vector<T>& data, BufferUsage usage = BufferUsage::STATIC_DRAW)
    : IndexBuffer(data.data(), static_cast<unsigned int>(data.size()), usage)
    {}
    
    ~IndexBuffer();
    
    GLuint getID() const { return m_RendererID; }
    void bind() const;
    void unbind() const;
    inline unsigned int getCount() const { return m_Count; }
};