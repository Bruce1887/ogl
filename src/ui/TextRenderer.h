#pragma once
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <memory>
#include "Shader.h"

struct Character {
    unsigned int textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class TextRenderer {
public:
    TextRenderer(int width, int height);
    void LoadFont(const std::string& fontPath, unsigned int fontSize);
    void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color);
    float GetTextWidth(const std::string& text, float scale);
    float GetTextHeight(float scale) const;
    void UpdateScreenSize(int width, int height);  // Update projection for new screen size
    
private:
    std::map<char, Character> m_chars;
    unsigned int m_VAO, m_VBO;
    std::shared_ptr<Shader> m_shader;
    int m_screenWidth, m_screenHeight;
};
