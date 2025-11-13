#pragma once

#include "Common.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Lighting.h"

// Abstract base class for anything that can be rendered
class Renderable
{
public:
    virtual void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) = 0;
    virtual ~Renderable() = default;

    // Potentially shared among multiple renderables.
    Shader *m_shaderRef;
    std::vector<Texture *> m_textureReferences;

    inline unsigned int getID() const { return m_ID; }

private:
    unsigned int m_ID = 0;

    GLuint m_VAO_ID = 0;
    // GLuint m_VBO_ID = 0;
    GLuint m_IBO_ID = 0;
    GLuint m_shader_ID = 0;
    std::vector<GLuint> m_texture_IDs;
};

// Anything that exists in the world (3D space) and has a transform
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
        return glm::vec3(m_transform[3]);
    }

    inline void setPosition(const glm::vec3 &pos)
    {        
        m_transform[3] = glm::vec4(pos, 1.0f);
    }
};

// Something that exists flat on the screen and can be rendered, e.g. HUD, health bar, score, menu, etc.
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

