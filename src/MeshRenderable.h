#pragma once

#include "Renderable.h"

// Holds the actual GPU data
class Mesh
{
public:
    Mesh(VertexArray *va, IndexBuffer *ib)
        : vertexArray(va), indexBuffer(ib)
    {
        std::cout << "Mesh created with VertexArray ID: " << vertexArray->getID() << std::endl;
        std::cout << "m_mesh->vertexArray->getCount(): " << vertexArray->getCount() << std::endl;
        if (indexBuffer)
            std::cout << "m_mesh->indexBuffer->getCount(): " << indexBuffer->getCount() << std::endl;
        else
            std::cout << "indexBuffer = nullptr" << std::endl;
    }

    // Shared by multiple renderables
    VertexArray *vertexArray;
    IndexBuffer *indexBuffer;
};

// Uses mesh data but doesn't own the heavy resources
class MeshRenderable : public WorldEntity
{
public:
    MeshRenderable(Mesh *mesh, Shader *shader)
        : m_mesh(mesh)
    {
        m_shaderRef = shader;
        setTransform(glm::mat4(1.0f));

        // rotation = glm::vec3(0.0f);         // Default rotation
        // scale = glm::vec3(1.0f);            // Default scale
    }

    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) override;

private:    
    Mesh *m_mesh; // Pointer to shared data
    // bool m_lightAffected = false; // maybe implement later (probably not)
};

