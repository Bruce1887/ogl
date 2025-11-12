#include "Renderable.h"

void MeshRenderable::render(glm::mat4 view, glm::mat4 projection)
{
    RenderingContext *rContext = RenderingContext::Current();
    // Bind shader if not already bound
    // std::cout << "correct shader bound?: " << (renderingContext->m_boundShader != m_shaderRef->getID()) << std::endl;
    if (rContext->m_boundShader != m_shaderRef->getID())
    {
        m_shaderRef->bind();
    }

    // Bind texture units if not already bound
    size_t numTextures = m_textureReferences.size();
    for (size_t i = 0; i < numTextures; i++)
    {
        Texture *texture = m_textureReferences[i];
        GLuint slot = texture->getSlot();
        GLuint texID = texture->getID();

        if (rContext->m_boundTextures[slot] != texID)
        {
            // TODO: optimize slot finding algorithm maybe

            // std::cout << "¤¤¤ TEXTURE NEEDS BINDING ¤¤¤" << std::endl;

            GLuint newslot = (slot + numTextures) % NUM_TEXTURE_UNITS;
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
    m_shaderRef->setUniform("view", view);
    m_shaderRef->setUniform("projection", projection);
    m_shaderRef->setUniform("model", getTransform());

    if (rContext->m_boundVAO != m_mesh->vertexArray->getID())
    {
        m_mesh->vertexArray->bind();
        rContext->m_boundVAO = m_mesh->vertexArray->getID();
    }

    if (rContext->m_boundIBO != m_mesh->indexBuffer->getID())
    {
        m_mesh->indexBuffer->bind();
        rContext->m_boundIBO = m_mesh->indexBuffer->getID();
    }

    GLCALL(glDrawElements(GL_TRIANGLES, m_mesh->indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));
}