#pragma once

#include <glm/glm.hpp>
#include <string>
#include <map>

// Forward declarations
class Shader;
class VertexArray;
class VertexBuffer;

// Character information structure
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2 Size;         // Size of glyph
    glm::ivec2 Bearing;      // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Horizontal offset to advance to next glyph
};

/**
 * @brief TextRenderer class for rendering text in the HUD
 * 
 * Uses stb_truetype for font loading and OpenGL for rendering.
 * Call init() after OpenGL context is created, then use RenderText() to draw text.
 */
class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();

    /**
     * @brief Initialize the text renderer with a font file
     * @param fontPath Path to the TTF font file
     * @param fontSize Size of the font in pixels
     * @param screenWidth Width of the screen for projection matrix
     * @param screenHeight Height of the screen for projection matrix
     * @return true if initialization was successful, false otherwise
     */
    bool init(const std::string& fontPath, unsigned int fontSize, int screenWidth, int screenHeight);

    /**
     * @brief Render a text string at the specified position
     * @param text The text string to render
     * @param x X position in screen coordinates (left edge)
     * @param y Y position in screen coordinates (baseline)
     * @param scale Scale factor for the text
     * @param color Color of the text (RGBA)
     */
    void RenderText(const std::string& text, float x, float y, float scale, const glm::vec4& color);

    /**
     * @brief Calculate the width of a text string
     * @param text The text string to measure
     * @param scale Scale factor for the text
     * @return Width in pixels
     */
    float GetTextWidth(const std::string& text, float scale) const;

    /**
     * @brief Check if the renderer is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return m_initialized; }

private:
    std::map<char, Character> m_Characters;
    Shader* m_textShader;
    VertexArray* m_VAO;
    VertexBuffer* m_VBO;
    
    int m_screenWidth;
    int m_screenHeight;
    bool m_initialized;
    
    /**
     * @brief Load font glyphs using stb_truetype
     * @param fontPath Path to the TTF font file
     * @param fontSize Font size in pixels
     * @return true if successful
     */
    bool loadFont(const std::string& fontPath, unsigned int fontSize);
};
