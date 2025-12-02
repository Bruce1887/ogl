#pragma once

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "basic_shapes.h"

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

    static std::shared_ptr<Mesh> createBoxMesh(){

        auto lightBox_VA_ptr = std::make_unique<VertexArray>();

        lightBox_VA_ptr->bind();
        auto lightBox_VB_ptr = std::make_unique<VertexBuffer>(BOX_VERTICES, BOX_VERTICES_SIZE, lightBox_VA_ptr.get());
        VertexBufferLayout lightBox_layout;
        lightBox_layout.push<float>(3); // position
        lightBox_VA_ptr->addBuffer(lightBox_VB_ptr.get(), lightBox_layout);
        auto lightBox_IBO_ptr = std::make_unique<IndexBuffer>(BOX_INDICES, BOX_INDICES_COUNT);
        
        auto lightBox_mesh_ptr = std::make_shared<Mesh>(std::move(lightBox_VA_ptr), std::move(lightBox_VB_ptr), std::move(lightBox_IBO_ptr));
        return lightBox_mesh_ptr;
    }

};