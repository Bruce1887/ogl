#pragma once
#include "MeshRenderable.h"

#include <assimp/Importer.hpp>	// C++ importer interface
#include <assimp/scene.h>		// Output data structure
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

    void addMeshRenderable(std::shared_ptr<MeshRenderable> mr) {
        m_meshRenderables.push_back(mr);
    }
    
    const std::vector<std::shared_ptr<MeshRenderable>>& getMeshRenderables() const {
        return m_meshRenderables;
    }

    bool m_hasTextureDiffuse = false;
    // bool m_hasTextureSpecular = false; // not implemented, not a priority either
    // bool m_hasTextureNormal = false; // not implemented, not a priority either
    // bool m_hasTextureHeight = false; // not implemented, not a priority either
};

class Model : public WorldEntity
{
private:
    std::shared_ptr<ModelData> m_modelData;    

public:
    // Constructor that loads from file (creates new ModelData)
    Model(const std::filesystem::path& path);
    
    // Constructor that shares existing ModelData
    Model(std::shared_ptr<ModelData> modelData);
    
    ~Model() = default;
    
    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) override;
    
    // Get the shared model data (useful for creating instances)
    std::shared_ptr<ModelData> getModelData() const { return m_modelData; }
};