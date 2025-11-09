/*
#pragma once

#include "Common.h"

class Renderable
{
public:
    virtual void Render() = 0;
    virtual ~Renderable() = default;
};

// Holds the actual GPU data
class Mesh
{
public:
    VertexBuffer* vertexBuffer;
    IndexBuffer* indexBuffer;
    // Shared by multiple renderables
};

// Uses mesh data but doesn't own the heavy resources
class MeshRenderable : public Renderable
{
public:
    MeshRenderable(Mesh* mesh, Transform transform)
        : m_mesh(mesh), m_transform(transform) {}
    
    void Render() override {
        // Use m_mesh data with m_transform
    }
    
private:
    Mesh* m_mesh;        // Pointer to shared data
    Transform m_transform; // Instance-specific data
};

class Scene
{
public:
	void AddRenderable(Renderable* renderable);
	void RemoveRenderable(Renderable* renderable);
	void RenderScene();
private:
	std::vector<Renderable*> m_renderables;
};
*/
