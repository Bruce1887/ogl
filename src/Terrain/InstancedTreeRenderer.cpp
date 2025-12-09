#include "InstancedTreeRenderer.h"
#include "MeshRenderable.h"
#include "RenderingContext.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

InstancedTreeRenderer::InstancedTreeRenderer()
{
}

InstancedTreeRenderer::~InstancedTreeRenderer()
{
    if (m_instanceVBO != 0)
    {
        glDeleteBuffers(1, &m_instanceVBO);
    }
}

void InstancedTreeRenderer::init(Model* treeModel)
{
    m_sourceModel = treeModel;

    // Create instanced shader - use non-textured version since tree model uses material colors only
    m_instancedShader = std::make_shared<Shader>();
    m_instancedShader->addShader("TreeInstanced.vert", ShaderType::VERTEX);
    m_instancedShader->addShader("PhongMTL_FOG.frag", ShaderType::FRAGMENT);
    m_instancedShader->createProgram();

    // Create instance VBO
    glGenBuffers(1, &m_instanceVBO);
}

void InstancedTreeRenderer::clearInstances()
{
    m_instanceTransforms.clear();
    m_dirty = true;
}

void InstancedTreeRenderer::addInstance(const glm::vec3& position, float scale, float rotationY)
{
    glm::mat4 transform(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::scale(transform, glm::vec3(scale));
    
    m_instanceTransforms.push_back(transform);
    m_dirty = true;
}

void InstancedTreeRenderer::uploadInstanceData()
{
    if (!m_dirty || m_instanceTransforms.empty())
        return;

    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, 
                 m_instanceTransforms.size() * sizeof(glm::mat4),
                 m_instanceTransforms.data(),
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_dirty = false;
}

void InstancedTreeRenderer::setFogUniforms(const glm::vec3& fogColor, float fogStart, float fogEnd)
{
    m_fogColor = fogColor;
    m_fogStart = fogStart;
    m_fogEnd = fogEnd;
}

void InstancedTreeRenderer::render(const glm::mat4& view, const glm::mat4& projection, PhongLightConfig* light)
{
    if (!m_sourceModel || m_instanceTransforms.empty())
        return;

    // Upload instance data if needed
    uploadInstanceData();

    m_instancedShader->bind();

    // Set common uniforms
    m_instancedShader->setUniform("u_view", view);
    m_instancedShader->setUniform("u_projection", projection);
    
    // Camera position for fog
    glm::vec3 camPos = glm::vec3(glm::inverse(view)[3]);
    m_instancedShader->setUniform("u_camPos", camPos);

    // Fog uniforms
    m_instancedShader->setUniform("u_fogColor", m_fogColor);
    m_instancedShader->setUniform("u_fogStart", m_fogStart);
    m_instancedShader->setUniform("u_fogEnd", m_fogEnd);

    // Light uniforms
    if (light)
    {
        m_instancedShader->setUniform("u_light_position", light->lightPosition);
        m_instancedShader->setUniform("u_light_ambient", light->ambientLight);
        m_instancedShader->setUniform("u_light_diffuse", light->diffuseLight);
        m_instancedShader->setUniform("u_light_specular", light->specularLight);
    }

    // Get the model's mesh renderables
    const auto& meshRenderables = m_sourceModel->getModelData()->getMeshRenderables();

    for (const auto& mr : meshRenderables)
    {
        // Get the mesh
        Mesh* mesh = mr->getMesh();
        if (!mesh) continue;

        // Copy material uniforms from the original mesh renderable (no textures for this model)
        for (const auto& kv : mr->getUniforms())
        {
            const std::string& uniformName = kv.first;
            const UniformValue& uniformValue = kv.second;
            std::visit([&, uniformName](auto&& arg) {
                m_instancedShader->setUniform(uniformName, arg);
            }, uniformValue);
        }

        // Bind the VAO
        mesh->vertexArray->bind();

        // Setup instance attribute pointers (mat4 = 4 vec4s)
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);

        // mat4 takes 4 vertex attribute slots (locations 5, 6, 7, 8)
        for (int i = 0; i < 4; i++)
        {
            GLuint loc = 5 + i;
            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), 
                                  (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(loc, 1); // One per instance
        }

        // Draw instanced
        if (mesh->indexBuffer)
        {
            mesh->indexBuffer->bind();
            glDrawElementsInstanced(GL_TRIANGLES, 
                                    mesh->indexBuffer->getCount(),
                                    GL_UNSIGNED_INT, 
                                    nullptr,
                                    static_cast<GLsizei>(m_instanceTransforms.size()));
        }
        else
        {
            glDrawArraysInstanced(GL_TRIANGLES, 0, 
                                  mesh->vertexArray->getCount(),
                                  static_cast<GLsizei>(m_instanceTransforms.size()));
        }

        // Cleanup instance attributes
        for (int i = 0; i < 4; i++)
        {
            glDisableVertexAttribArray(5 + i);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
