#pragma once

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

#include <optional>

// Holds the actual GPU data
class Mesh
{
public:
    Mesh(std::unique_ptr<VertexArray> va, std::unique_ptr<VertexBuffer> vb, std::unique_ptr<IndexBuffer> ib)
        : vertexArray(std::move(va)), vertexBuffer(std::move(vb)), indexBuffer(std::move(ib))
    {}

    Mesh(std::unique_ptr<VertexArray> va, std::unique_ptr<VertexBuffer> vb)   
        : vertexArray(std::move(va)), vertexBuffer(std::move(vb)), indexBuffer(nullptr)
    {}


    std::unique_ptr<VertexArray> vertexArray;
    std::unique_ptr<VertexBuffer> vertexBuffer; // we just need to keep a reference to the vertexbuffer to avoid it being deleted (and freed on the GPU)
    std::unique_ptr<IndexBuffer> indexBuffer;
};