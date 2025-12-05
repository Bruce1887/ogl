#include "Scene.h"
#include "Renderable.h"
#include "VertexBufferLayout.h"
#include "MeshRenderable.h"

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

void Scene::clearRenderables()
{
	m_renderables.clear();
}

void Scene::renderScene()
{
	// Set up view and projection matrices from the active camera
	glm::mat4 view = m_activeCamera.getViewMatrix();
	glm::mat4 projection = m_activeCamera.getProjectionMatrix();

	

	// Render each object in the scene
	for (auto &r : m_renderables)
	{
		r->render(view, projection, &m_lightSource.config);
	}

	// render the light source's visual representation if any
	if (m_lightSource.visualRepresentation != nullptr)
	{
		m_lightSource.visualRepresentation->setPosition(m_lightSource.config.lightPosition);
		m_lightSource.visualRepresentation->render(view, projection, nullptr);
	}

	// Render the skybox cube (it handles its own glDepthFunc switch)
	if (m_skybox)
		m_skybox->render(view, projection, nullptr);
		
	// Swap buffers
	glfwSwapBuffers(g_window);
}
