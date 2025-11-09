// #include "Scene.h"

// #include <algorithm>

// void Scene::AddRenderable(Renderable* renderable)
// {
// 	m_renderables.push_back(renderable);
// }

// void Scene::RemoveRenderable(Renderable* renderable)
// {
// 	auto it = std::find(m_renderables.begin(), m_renderables.end(), renderable);
// 	if (it != m_renderables.end())
// 	{
// 		m_renderables.erase(it);
// 	}
// }
// void Scene::RenderScene()
// {
// 	for (auto& renderable : m_renderables)
// 	{
// 		renderable->Render();
// 	}
// }
