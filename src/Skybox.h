#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

class Shader; 

class Skybox {
public:
    Skybox(const std::vector<std::string>& faces);
    ~Skybox();
    void render(Shader& shader, const glm::mat4& view, const glm::mat4& projection);
    void setupSkybox();
private:
    unsigned int m_textureID; 
    unsigned int m_vao, m_vbo; 
    void loadCubemap(const std::vector<std::string>& faces);
};