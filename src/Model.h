#pragma once
#include "MeshRenderable.h"

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include <filesystem>
#include <memory>
#include <vector>

class ModelData
{
private:
    std::vector<std::shared_ptr<MeshRenderable>> m_meshRenderables;

public:
    ModelData() = default;
    ~ModelData();

    void addMeshRenderable(std::shared_ptr<MeshRenderable> mr)
    {
        m_meshRenderables.push_back(mr);
    }

    const std::vector<std::shared_ptr<MeshRenderable>> &getMeshRenderables() const
    {
        return m_meshRenderables;
    }

    bool m_hasTextureDiffuse = false;
    // bool m_hasTextureSpecular = false; // not implemented, not a priority either
    // bool m_hasTextureNormal = false; // not implemented, not a priority either
    // bool m_hasTextureHeight = false; // not implemented, not a priority either
};

class Model : public WorldEntity
{
public:
    // Constructor that loads from file (creates new ModelData)
    Model(const std::filesystem::path &path);

    static Model copyFrom(const Model *other)
    {
        return Model(other->m_modelData);
    }

    ~Model() = default;

    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) override;
    
    void setFogUniforms(const glm::vec3& fogColor, float fogStart, float fogEnd)
    {
        for (auto &mr : m_modelData->getMeshRenderables())
        {
            mr->setUniform("u_fogColor", fogColor);
            mr->setUniform("u_fogStart", fogStart);
            mr->setUniform("u_fogEnd", fogEnd);
        }
    }

private:
    // Get the shared model data (useful for creating instances)

    std::shared_ptr<ModelData> m_modelData;

    // Constructor that shares existing ModelData
    Model(std::shared_ptr<ModelData> modelData);
};