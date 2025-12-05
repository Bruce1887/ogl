#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "MeshRenderable.h"
#include "Renderable.h"
#include "Shader.h"

class Skybox : public Renderable
{
public:
    Skybox(const std::vector<std::string> &faces);
    ~Skybox();
    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) override;
    
    private:
    std::shared_ptr<Shader> m_shader;
    unsigned int m_cubemapTextureID;
    unsigned int m_vao, m_vbo;
    
    void setupSkybox();
    void loadCubemap(const std::vector<std::string> &faces);
    void setUpShader();

};