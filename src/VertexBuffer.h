#pragma once

#include "Common.h"
#include <vector>

class VertexBuffer
{
private:
    unsigned int m_RendererID; // Unique ID for the buffer

public:
    VertexBuffer(const void* data, unsigned int size, BufferUsage usage = BufferUsage::STATIC_DRAW);

    template<typename T>
    VertexBuffer(const std::vector<T>& data, BufferUsage usage = BufferUsage::STATIC_DRAW)
        : VertexBuffer(data.data(), static_cast<unsigned int>(data.size() * sizeof(T)), usage)
    {}

    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
};
