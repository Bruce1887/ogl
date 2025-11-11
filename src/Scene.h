#pragma once

#include "Common.h"
#include "Renderable.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"

class Scene
{
public:
    Scene(const Camera &defaultCamera)
        : m_activeCamera(defaultCamera)
    {
    }

    ~Scene() = default;

    void addRenderable(Renderable *renderable);
    void removeRenderable(Renderable *renderable);
    void renderScene();

    Camera m_activeCamera;
    int nextCamera();
    int addCamera(const Camera &camera);
    void removeCamera(const Camera &camera);
    void setActiveCamera(int index);

private:
    std::vector<Renderable *> m_renderables;
    std::vector<Camera> m_cameras;
};