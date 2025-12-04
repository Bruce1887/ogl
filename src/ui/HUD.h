#pragma once

#include <glm/glm.hpp>
#include <string>

// Include the full class definitions
#include "../game/Player.h"
#include "../game/GameClock.h"

// Forward declarations for rendering components
class Shader;
class VertexArray;
class VertexBuffer;
class TextRenderer;

class HUD {
public:
    HUD(game::Player* player, game::GameClock* clock, int screenWidth, int screenHeight);
    ~HUD();

    void Draw();
    
    /**
     * @brief Initialize text rendering with a font file
     * @param fontPath Path to the TTF font file
     * @param fontSize Size of the font in pixels
     * @return true if successful
     */
    bool initTextRenderer(const std::string& fontPath, unsigned int fontSize);

private:
    game::Player* m_player;
    game::GameClock* m_clock;

    int m_screenWidth;
    int m_screenHeight;

    Shader* m_hudShader;
    VertexArray* m_quadVAO;
    VertexBuffer* m_quadVBO;
    
    TextRenderer* m_textRenderer;

    void DrawRect(float x, float y, float width, float height, const glm::vec4& color);
    void DrawVitalsBars();
    void DrawClockDisplay();
    void DrawText(const std::string& text, float x, float y, const glm::vec4& color, float scale = 1.0f);
    void SetupQuadRendering();
};

