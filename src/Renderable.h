#pragma once

#include "Common.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

class Renderable
{
public:
    virtual void render(glm::mat4 view, glm::mat4 projection) = 0;
    virtual ~Renderable() = default;

    // Potentially shared among multiple renderables.
    Shader *m_shaderRef;

private:
    unsigned int VAO_ID;
    // unsigned int VBO_ID;
    unsigned int IBO_ID;
    unsigned int shader_ID;
    std::vector<unsigned int> texture_IDs;
};

// Something that exists in the world and can be rendered, e.g. a player, an enemy, a tree, etc.
class WorldEntity : public Renderable
{
    // TODO: add rotation, scale, and whatever else is needed
private:
    glm::mat4 m_transform = glm::mat4(1.0f); // Initialize to identity
public:
    inline glm::mat4 getTransform() const
    {
        return m_transform;
    }

    inline void setTransform(const glm::mat4 &transform)
    {
        m_transform = transform;
    }

    inline glm::vec3 getPosition() const
    {
        // Get the translation column (column 3)
        return glm::vec3(m_transform[3]);
    }

    inline void setPosition(const glm::vec3 &pos)
    {
        // Set the translation column
        m_transform[3] = glm::vec4(pos, 1.0f);
    }
};

// Something that exists in the HUD and can be rendered, e.g. health bar, score, menu, etc.
class HUDEntity : public Renderable
{
public:
    glm::vec2 m_position; // 2D position on screen
    glm::vec2 m_size;     // Size on screen
};

// Holds the actual GPU data
class Mesh
{
public:
    Mesh(VertexArray *va, IndexBuffer *ib)
        : vertexArray(va), indexBuffer(ib) {}

    VertexArray *vertexArray;
    IndexBuffer *indexBuffer;
    // Shared by multiple renderables
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

    void render(glm::mat4 view, glm::mat4 projection) override;

private:
    Mesh *m_mesh; // Pointer to shared data
};