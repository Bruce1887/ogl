#pragma once
#include "MeshRenderable.h"

#include <assimp/Importer.hpp>	// C++ importer interface
#include <assimp/scene.h>		// Output data structure
#include <assimp/postprocess.h> // Post processing flags

class Model : public WorldEntity
{
    std::vector<MeshRenderable *> m_meshRenderables;

public:
    Model(const std::string& path);
    ~Model();
    virtual glm::mat4 getTransform() const override
    {
        return m_transform;
    }
    virtual void setTransform(const glm::mat4 &transform) override
    {
        for (auto &mr : m_meshRenderables)
        {
            mr->setTransform(transform);
        }
        m_transform = transform;
    }
    virtual glm::vec3 getPosition() const override
    {
        return glm::vec3(m_transform[3]);
    }
    virtual void setPosition(const glm::vec3 &pos) override
    {
        for (auto &mr : m_meshRenderables)
        {
            mr->setPosition(pos);
        }
        m_transform[3] = glm::vec4(pos, 1.0f);
    }
    
    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) override;
};