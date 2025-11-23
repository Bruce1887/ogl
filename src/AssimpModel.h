#pragma once

#include "Renderable.h"
#include "MeshRenderable.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "Shader.h"

#include <assimp/scene.h>
#include <vector>
#include <memory>

struct MeshData
{
    std::unique_ptr<VertexArray> va;
    std::unique_ptr<VertexBuffer> vb;
    std::unique_ptr<IndexBuffer> ib;
    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<MeshRenderable> renderable;
};

class AssimpModel : public WorldEntity
{
public:
    AssimpModel(const aiScene* scene, Shader* shader);
    ~AssimpModel() = default;

    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig* phongLight) override;

private:
    std::vector<MeshData> m_meshes;

    void processMesh(const aiMesh* mesh, Shader* shader);
    void extractMeshData(const aiMesh* mesh, 
                        std::vector<float>& vertices,
                        std::vector<unsigned int>& indices);
};

