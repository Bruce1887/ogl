#include "TextRenderer.h"

#include "../Common.h"
#include "../Shader.h"
#include "../VertexArray.h"
#include "../VertexBuffer.h"
#include "../VertexBufferLayout.h"

#include <vendor/stb_image/stb_truetype.h>
#include <fstream>
#include <vector>

// Constants for text quad vertex data
// Each character quad has 6 vertices (2 triangles), each with 4 floats (x, y, tex_s, tex_t)
static constexpr int VERTICES_PER_QUAD = 6;
static constexpr int FLOATS_PER_VERTEX = 4;
static constexpr size_t QUAD_BUFFER_SIZE = sizeof(float) * VERTICES_PER_QUAD * FLOATS_PER_VERTEX;

TextRenderer::TextRenderer()
    : m_textShader(nullptr)
    , m_VAO(nullptr)
    , m_VBO(nullptr)
    , m_screenWidth(0)
    , m_screenHeight(0)
    , m_initialized(false)
{
}

TextRenderer::~TextRenderer()
{
    // Clean up OpenGL resources
    for (auto& c : m_Characters) {
        glDeleteTextures(1, &c.second.TextureID);
    }
    delete m_textShader;
    delete m_VAO;
    delete m_VBO;
}

bool TextRenderer::init(const std::string& fontPath, unsigned int fontSize, int screenWidth, int screenHeight)
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Initialize shader
    m_textShader = new Shader();
    m_textShader->addShader("Text.vert", ShaderType::VERTEX);
    m_textShader->addShader("Text.frag", ShaderType::FRAGMENT);
    m_textShader->createProgram();

    // Load font
    if (!loadFont(fontPath, fontSize)) {
        return false;
    }

    // Setup VAO/VBO for text quads
    m_VAO = new VertexArray();
    
    // Reserve space for one character quad - we'll update this dynamically when rendering
    m_VBO = new VertexBuffer(nullptr, static_cast<unsigned int>(QUAD_BUFFER_SIZE), m_VAO, BufferUsage::DYNAMIC_DRAW);
    
    VertexBufferLayout layout;
    layout.push<float>(FLOATS_PER_VERTEX); // vec4 (pos.x, pos.y, tex.x, tex.y)
    m_VAO->addBuffer(m_VBO, layout);

    m_initialized = true;
    return true;
}

bool TextRenderer::loadFont(const std::string& fontPath, unsigned int fontSize)
{
    // Read font file
    std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "TextRenderer: Failed to open font file: " << fontPath << std::endl;
        return false;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<unsigned char> fontBuffer(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(fontBuffer.data()), fileSize)) {
        std::cerr << "TextRenderer: Failed to read font file" << std::endl;
        return false;
    }
    file.close();

    // Initialize stb_truetype
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontBuffer.data(), stbtt_GetFontOffsetForIndex(fontBuffer.data(), 0))) {
        std::cerr << "TextRenderer: Failed to initialize font" << std::endl;
        return false;
    }

    // Get font scale for desired pixel height
    float scale = stbtt_ScaleForPixelHeight(&font, static_cast<float>(fontSize));

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load ASCII characters 32-126
    for (unsigned char c = 32; c < 127; c++) {
        int width, height, xoff, yoff;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, c, &width, &height, &xoff, &yoff);

        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            width,
            height,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            bitmap
        );

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Get advance width
        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&font, c, &advanceWidth, &leftSideBearing);

        // Store character for later use
        Character character = {
            texture,
            glm::ivec2(width, height),
            glm::ivec2(xoff, yoff),
            static_cast<unsigned int>(static_cast<float>(advanceWidth) * scale)
        };
        m_Characters.insert(std::pair<char, Character>(static_cast<char>(c), character));

        stbtt_FreeBitmap(bitmap, nullptr);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void TextRenderer::RenderText(const std::string& text, float x, float y, float scale, const glm::vec4& color)
{
    if (!m_initialized) return;

    // Enable blending for text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Set up shader
    m_textShader->bind();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(m_screenWidth), 
                                       0.0f, static_cast<float>(m_screenHeight));
    m_textShader->setUniform("u_Projection", projection);
    m_textShader->setUniform("u_textColor", color);
    m_textShader->setUniform("u_textTexture", 0);

    glActiveTexture(GL_TEXTURE0);
    m_VAO->bind();

    // Iterate through all characters
    for (char c : text) {
        if (m_Characters.find(c) == m_Characters.end()) {
            continue; // Skip unknown characters
        }

        Character ch = m_Characters[c];

        // stb_truetype uses different coordinate system - yoff is typically negative
        // and represents the top of the character relative to baseline
        float xpos = x + static_cast<float>(ch.Bearing.x) * scale;
        // For bottom-left origin (OpenGL default with our projection),
        // we need to offset y properly
        float ypos = y - (static_cast<float>(ch.Size.y) + static_cast<float>(ch.Bearing.y)) * scale;

        float w = static_cast<float>(ch.Size.x) * scale;
        float h = static_cast<float>(ch.Size.y) * scale;

        // Update VBO for each character
        float vertices[VERTICES_PER_QUAD][FLOATS_PER_VERTEX] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        m_VBO->bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, VERTICES_PER_QUAD);

        // Advance cursor for next glyph
        x += static_cast<float>(ch.Advance) * scale;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

float TextRenderer::GetTextWidth(const std::string& text, float scale) const
{
    float width = 0.0f;
    for (char c : text) {
        auto it = m_Characters.find(c);
        if (it != m_Characters.end()) {
            width += static_cast<float>(it->second.Advance) * scale;
        }
    }
    return width;
}
