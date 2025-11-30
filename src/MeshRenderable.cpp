#include "MeshRenderable.h"

void MeshRenderable::render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight)
{
    RenderingContext *rContext = RenderingContext::Current();

    // Bind shader if not already bound
    if (m_shaderRef->getID() != rContext->m_boundShader)
    {        
        m_shaderRef->bind();
    }

    // Bind texture units if not already bound
    size_t numTextures = m_textureReferences.size();
    for (size_t i = 0; i < numTextures; i++)
    {
        Texture *texture = m_textureReferences[i].get();
        GLuint slot = texture->getSlot();
        GLuint texID = texture->getID();

        if (rContext->m_boundTextures[slot] != texID)
        {
            // TODO: optimize slot finding algorithm maybe
            // We need to bind the texture to some slot, and then update the uniform
            GLuint newslot = (slot + numTextures) % REQUIRED_NUM_TEXTURE_UNITS;
            texture->bindNew(newslot);
        }

        /* We always need to update the uniform to the correct slot in case it was changed ( I THINK )
        Alternatively we could implement tracking such that each shader knows which slots its
        uniforms are bound to.
        Another approach is to compile a separate shader program for each MeshRenderable instance.
        */
        m_shaderRef->setUniform(texture->targetUniform, texture->getSlot());
    }

    // Set other uniforms
    m_shaderRef->setUniform("u_view", view);
    m_shaderRef->setUniform("u_projection", projection);
    m_shaderRef->setUniform("u_model", getTransform());

    if (phongLight != nullptr)
    {
        m_shaderRef->setUniform("u_light_position", phongLight->lightPosition);
        m_shaderRef->setUniform("u_light_ambient", phongLight->ambientLight);
        m_shaderRef->setUniform("u_light_diffuse", phongLight->diffuseLight);
        m_shaderRef->setUniform("u_light_specular", phongLight->specularLight);
        glm::vec3 camPos = glm::vec3(glm::inverse(view)[3]);
        m_shaderRef->setUniform("u_camPos", camPos);
    }

    if (rContext->m_boundVAO != m_mesh->vertexArray->getID())
    {
        m_mesh->vertexArray->bind();
        rContext->m_boundVAO = m_mesh->vertexArray->getID();
    }

    if (m_mesh->indexBuffer != nullptr)
    {
        // std::cout << "Drawing indexed mesh." << std::endl;
        if (rContext->m_boundIBO != m_mesh->indexBuffer->getID())
        {
            // std::cout << "Binding index buffer object." << std::endl;
            m_mesh->indexBuffer->bind();
            rContext->m_boundIBO = m_mesh->indexBuffer->getID();
        }

        int count = m_mesh->indexBuffer->getCount();
        // std::cout << "Index count: " << count << std::endl;

        // std::cout << "Issuing glDrawElements call." << std::endl;
        GLCALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr));
        // std::cout << "glDrawElements call completed." << std::endl;
    }
    else
    {        
        GLCALL(glDrawArrays(GL_TRIANGLES, 0, m_mesh->vertexArray->getCount()));
    }
}