#pragma once

#include "Common.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"

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
public:
    glm::vec3 m_position;

    glm::mat4 getTransform()
    {
        return glm::translate(glm::mat4(1.0f), m_position);

        // glm::mat4 model_matrix = glm::rotate(
        //     glm::translate(glm::mat4(1.0f), m_position),
        //     glm::radians(10.0f), 
        //     glm::vec3(1.0f, 0.0f, 1.0f));
        // return model_matrix;
    }

    // glm::vec3 m_rotation;
    // glm::vec3 m_scale;
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
    Mesh(VertexBuffer *vb, IndexBuffer *ib)
        : vertexBuffer(vb), indexBuffer(ib) {}

    VertexBuffer *vertexBuffer;
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
        m_position = glm::vec3(0.0f);

        // rotation = glm::vec3(0.0f);         // Default rotation
        // scale = glm::vec3(1.0f);            // Default scale
    }

    void render(glm::mat4 view, glm::mat4 projection) override;

private:
    Mesh *m_mesh; // Pointer to shared data
};

class Scene
{
public:
    Scene(const Camera &defaultCamera)
        : m_activeCamera(defaultCamera)
    {
    }

    ~Scene() = default;

    void addRenderable(Renderable *renderable);
    void removeRenderable(Renderable *renderable);
    void renderScene();

    Camera m_activeCamera;
    int nextCamera();
    int addCamera(const Camera &camera);
    void removeCamera(const Camera &camera);
    void setActiveCamera(int index);

private:
    std::vector<Renderable *> m_renderables;
    std::vector<Camera> m_cameras;
};