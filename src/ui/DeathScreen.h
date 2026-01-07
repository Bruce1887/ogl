#pragma once

#include "UIConfig.h"

#include <glm/glm.hpp>
#include <string>
#include <functional>
#include <memory>

class Shader;
class VertexArray;
class VertexBuffer;
class TextRenderer;
struct PhongLightConfig;

namespace ui
{

class DeathScreen
{
public:
    DeathScreen(int screenWidth, int screenHeight);
    ~DeathScreen();

    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig* phongLight);
    void update(float deltaTime);

    // Input handling
    void handleMouseMove(double mouseX, double mouseY);
    void handleMouseClick(double mouseX, double mouseY, bool pressed);
    void handleCharInput(unsigned int codepoint);
    void handleKeyInput(int key, int action);

    // Set the score to display
    void setScore(int score) { m_score = score; }

    // Callback when submit is clicked (passes player name and score)
    std::function<void(const std::string& playerName, int score)> onSubmitScore;
    // Callback when continue is clicked (skip submission)
    std::function<void()> onContinueClicked;

    void updateScreenSize(int width, int height);

private:
    int m_screenWidth;
    int m_screenHeight;
    int m_score = 0;

    std::shared_ptr<Shader> m_shader;
    std::unique_ptr<VertexArray> m_quadVAO;
    std::unique_ptr<VertexBuffer> m_quadVBO;
    std::unique_ptr<TextRenderer> m_textRenderer;

    // Text input state
    std::string m_playerName;
    static constexpr size_t MAX_NAME_LENGTH = 20;
    bool m_textInputFocused = true;
    float m_cursorBlinkTimer = 0.0f;
    bool m_showCursor = true;

    // Text input box dimensions
    float m_inputBoxX, m_inputBoxY, m_inputBoxWidth, m_inputBoxHeight;

    // Submit button state
    float m_submitButtonX, m_submitButtonY, m_submitButtonWidth, m_submitButtonHeight;
    bool m_submitButtonHovered = false;
    bool m_submitButtonPressed = false;

    // Continue button state (skip submission)
    float m_buttonX, m_buttonY, m_buttonWidth, m_buttonHeight;
    bool m_buttonHovered = false;
    bool m_buttonPressed = false;

    // Animation
    float m_fadeAlpha = 0.0f;
    float m_displayTime = 0.0f;

    void DrawRect(float x, float y, float width, float height, const glm::vec4& color, float zOffset = 0.0f);
    void DrawText(const std::string& text, float x, float y, float scale, const glm::vec4& color);
    void DrawTextLeft(const std::string& text, float x, float y, float scale, const glm::vec4& color);
    void SetupQuadRendering();
};

} // namespace ui
