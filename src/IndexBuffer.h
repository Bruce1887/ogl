#pragma once

#include "Common.h"

class IndexBuffer
{
private:
    unsigned int m_RendererID; // Unique ID for the buffer
    unsigned int m_Count;      // Number of indices
public:
    IndexBuffer(const unsigned int *data, unsigned int count, BufferUsage usage = BufferUsage::STATIC_DRAW);

    template<typename T>
    IndexBuffer(const std::vector<T>& data, BufferUsage usage = BufferUsage::STATIC_DRAW)
        : IndexBuffer(data.data(), static_cast<unsigned int>(data.size()), usage)
    {}

    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;
    inline unsigned int GetCount() const { return m_Count; }
};