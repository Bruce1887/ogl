#include "Scene.h"
#include "Renderable.h"

#include <algorithm>

void Scene::addRenderable(Renderable *renderable)
{
	m_renderables.push_back(renderable);
}

void Scene::removeRenderable(Renderable *renderable)
{
	auto it = std::find(m_renderables.begin(), m_renderables.end(), renderable);
	if (it != m_renderables.end())
	{
		m_renderables.erase(it);
	}
}

void Scene::renderScene()
{
	glm::mat4 view = m_activeCamera.GetViewMatrix();
	glm::mat4 projection = m_activeCamera.GetProjectionMatrix();
	for (auto &r : m_renderables)
	{
		r->render(view, projection);
	}
	glfwSwapBuffers(window);
	glfwPollEvents();

}

void MeshRenderable::render(glm::mat4 view, glm::mat4 projection)
    {        
        m_shaderRef->bind();
        m_shaderRef->setUniform("view", view);
        m_shaderRef->setUniform("projection", projection);
        m_shaderRef->setUniform("model", getTransform());
		

		
        m_mesh->vertexArray->bind();
        m_mesh->indexBuffer->bind();		
		


        GLCALL(glDrawElements(GL_TRIANGLES, m_mesh->indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));
    }