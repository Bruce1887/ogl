#pragma once

#include "VertexArray.h"
#include "IndexBuffer.h"
#include <optional>

// Holds the actual GPU data
class Mesh
{
public:
    Mesh(VertexArray&& va, IndexBuffer&& ib)
        : vertexArray(std::move(va)), indexBuffer(std::move(ib))
    {}

    Mesh(VertexArray&& va)
        : vertexArray(std::move(va)), indexBuffer(std::nullopt)
    {}

    VertexArray vertexArray;
    std::optional<IndexBuffer> indexBuffer;
};