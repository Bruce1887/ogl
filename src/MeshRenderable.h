#pragma once

#include "Renderable.h"
#include "Mesh.h"

// Uses mesh data but doesn't own the heavy resources
class MeshRenderable : public WorldEntity
{
public:
    MeshRenderable(std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader)        
    {
        m_mesh = mesh;
        m_shaderRef = shader;
        setTransform(glm::mat4(1.0f));
    }

    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) override;

    // Get underlying mesh (for instanced rendering)
    Mesh* getMesh() const { return m_mesh.get(); }

private:
    std::shared_ptr<Mesh> m_mesh; // Pointer to shared data    
    // bool m_lightAffected = false; // maybe implement later (probably not)
};
