#pragma once

#include <glm/glm.hpp>
#include <string>
#include <functional>
#include <memory>
#include <vector>

// Forward declarations
class Shader;
class VertexArray;
class VertexBuffer;
class TextRenderer;
struct PhongLightConfig;

namespace ui
{

// Button for pause menu
struct PauseButton
{
    std::string label;
    float x, y;
    float width, height;
    glm::vec4 normalColor{0.2f, 0.2f, 0.3f, 0.9f};
    glm::vec4 hoverColor{0.3f, 0.3f, 0.5f, 1.0f};
    glm::vec4 activeColor{0.4f, 0.4f, 0.6f, 1.0f};
    std::function<void()> onClick;

    bool isHovered = false;
    bool isPressed = false;

    bool contains(float px, float py) const
    {
        return px >= x && px <= (x + width) &&
               py >= y && py <= (y + height);
    }
};

// Pause menu - overlay during gameplay
class PauseMenu
{
public:
    PauseMenu(int screenWidth, int screenHeight);
    ~PauseMenu();

    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig* phongLight);

    // Input handling
    void handleMouseMove(double mouseX, double mouseY);
    void handleMouseClick(double mouseX, double mouseY, bool pressed);

    // Button callbacks
    std::function<void()> onResumeClicked;
    std::function<void()> onSettingsClicked;
    std::function<void()> onQuitClicked;

    void updateScreenSize(int width, int height);

private:
    int m_screenWidth;
    int m_screenHeight;

    std::shared_ptr<Shader> m_shader;
    std::unique_ptr<VertexArray> m_quadVAO;
    std::unique_ptr<VertexBuffer> m_quadVBO;
    std::unique_ptr<TextRenderer> m_textRenderer;

    std::vector<PauseButton> m_buttons;

    // Drawing helpers
    void DrawRect(float x, float y, float width, float height, const glm::vec4& color, float zOffset = 0.0f);
    void DrawButton(const PauseButton& button);
    void DrawText(const std::string& text, float x, float y, float scale, const glm::vec4& color);
    void SetupQuadRendering();
    void InitializeButtons();
};

} // namespace ui
