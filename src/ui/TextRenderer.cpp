#include "TextRenderer.h"
#include <ft2build.h>
#include FT_FREETYPE_H

TextRenderer::TextRenderer(int width, int height)
{
    m_shader = std::make_shared<Shader>();
    m_shader->addShader("text.vert", ShaderType::VERTEX);
    m_shader->addShader("text.frag", ShaderType::FRAGMENT);
    m_shader->createProgram();

    m_shader->bind();
    glm::mat4 proj = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
    m_shader->setUniform("projection", proj);

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*4, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
    glEnableVertexAttribArray(0);
}

void TextRenderer::LoadFont(const std::string& path, unsigned int size)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        throw std::runtime_error("Failed to init FreeType");

    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face))
        throw std::runtime_error("Failed to load font");

    FT_Set_Pixel_Sizes(face, 0, size);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 32; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character ch = {
            tex,
            { face->glyph->bitmap.width, face->glyph->bitmap.rows },
            { face->glyph->bitmap_left, face->glyph->bitmap_top },
            (unsigned int)face->glyph->advance.x
        };

        m_chars.insert({ c, ch });
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void TextRenderer::RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color)
{
    // Save current GL state
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    m_shader->bind();
    m_shader->setUniform("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);

    float originalX = x;
    
    for (auto c : text)
    {        
        if (m_chars.find(c) == m_chars.end())
        {
            x += 20.0f * scale; // Space for missing chars
            continue;
        }
            
        const Character& ch = m_chars.at(c);

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - ch.bearing.y * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float verts[6][4] = {
            { xpos,      ypos + h,   0.0f, 1.0f },
            { xpos,      ypos,       0.0f, 0.0f },
            { xpos + w,  ypos,       1.0f, 0.0f },

            { xpos,      ypos + h,   0.0f, 1.0f },
            { xpos + w,  ypos,       1.0f, 0.0f },
            { xpos + w,  ypos + h,   1.0f, 1.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;
    }
    
    // Restore depth test
    if (depthTestEnabled)
        glEnable(GL_DEPTH_TEST);
}

float TextRenderer::GetTextWidth(const std::string& text, float scale)
{
    float width = 0.0f;
    for (char c : text)
    {
        if (m_chars.count(c) == 0)
            continue;

        const Character& ch = m_chars.at(c);
        width += (ch.advance >> 6) * scale; // FreeType uses 1/64th pixels
    }
    return width;
}

float TextRenderer::GetTextHeight(float scale) const
{
    float maxAscent = 0.0f;
    float maxDescent = 0.0f;

    for (const auto& [c, ch] : m_chars)
    {
        maxAscent = std::max(maxAscent, ch.bearing.y * scale);
        maxDescent = std::max(maxDescent, (ch.size.y - ch.bearing.y) * scale);
    }

    return maxAscent + maxDescent;
}