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
    Skybox();
    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) override;
    
    private:
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<MeshRenderable> m_skybox_mr;
    std::shared_ptr<Texture> m_cubemapTexture;
    
    void setUpMR();
    void loadCubemap(const std::vector<std::string> &faces);
    void setUpShader();

};