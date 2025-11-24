#pragma once

#include "Common.h"
#include "Renderable.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Lighting.h"

class Scene
{
public:
Scene(Camera defaultCamera, LightSource lightSource)
: m_activeCamera(defaultCamera), m_lightSource(lightSource)
    {  

    }

    ~Scene() = default;

    void addRenderable(Renderable *renderable);
    void removeRenderable(Renderable *renderable);
    void clearRenderables(); // Clear all renderables from scene
    void renderScene();

    Camera m_activeCamera;
    int nextCamera();
    int addCamera(const Camera &camera);
    void removeCamera(const Camera &camera);
    void setActiveCamera(int index);

    // One single light for now (maybe ever)
    LightSource m_lightSource;        
    glm::vec3 getLightPosition() const { return m_lightSource.config.lightPosition; }
    glm::vec3 getLightAmbient() const { return m_lightSource.config.ambientLight; }
    glm::vec3 getLightDiffuse() const { return m_lightSource.config.diffuseLight; }
    glm::vec3 getLightSpecular() const { return m_lightSource.config.specularLight; }

    inline void tick()
    {
        // Clear the screen at the start of each frame
        GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // Poll for and process events
        glfwPollEvents();
    }

private:    
    // A list of all renderables in the scene
    std::vector<Renderable *> m_renderables;
    unsigned int m_nextVacantID = 0; // For assigning unique IDs to renderables
    
    // A list of all cameras in the scene //TODO: implement this one day
    std::vector<Camera> m_cameras;
};