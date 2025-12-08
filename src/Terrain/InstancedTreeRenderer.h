#pragma once

#include "Common.h"
#include "Model.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "Lighting.h"

#include <vector>
#include <memory>
#include <glm/glm.hpp>

/**
 * @brief Renders many instances of a tree model in a single draw call using GPU instancing.
 * 
 * This class stores instance transforms and uploads them to the GPU, allowing
 * thousands of trees to be rendered with just one draw call per mesh.
 */
class InstancedTreeRenderer
{
public:
    InstancedTreeRenderer();
    ~InstancedTreeRenderer();

    // Initialize with the tree model to instance
    void init(Model* treeModel);

    // Clear all instances
    void clearInstances();

    // Add a tree instance at the given position
    void addInstance(const glm::vec3& position, float scale = 1.0f, float rotationY = 0.0f);

    // Upload instance data to GPU (call after adding all instances)
    void uploadInstanceData();

    // Render all instances
    void render(const glm::mat4& view, const glm::mat4& projection, PhongLightConfig* light);

    // Set fog uniforms
    void setFogUniforms(const glm::vec3& fogColor, float fogStart, float fogEnd);

    // Get instance count
    size_t getInstanceCount() const { return m_instanceTransforms.size(); }

private:
    // Source model data (shared, not owned)
    Model* m_sourceModel = nullptr;

    // Instance transforms (mat4 stored as 16 floats)
    std::vector<glm::mat4> m_instanceTransforms;

    // GPU buffer for instance data
    GLuint m_instanceVBO = 0;

    // Instanced shader
    std::shared_ptr<Shader> m_instancedShader;

    // Fog parameters
    glm::vec3 m_fogColor = glm::vec3(0.5f, 0.9f, 0.95f);
    float m_fogStart = 80.0f;
    float m_fogEnd = 100.0f;

    // Whether instance data needs re-upload
    bool m_dirty = true;
};
