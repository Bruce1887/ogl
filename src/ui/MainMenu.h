#pragma once

#include "GameState.h"
#include <glm/glm.hpp>
#include <string>
#include <functional>
#include <memory>
#include "../Renderable.h"

// Forward declarations
class Shader;
class VertexArray;
class VertexBuffer;
class Skybox;
class TextRenderer;
struct PhongLightConfig;

namespace ui
{

// Represents a clickable UI button
struct MenuButton
{
    std::string label;
    float x, y;           // position (top-left corner in pixels)
    float width, height;  // dimensions
    glm::vec4 normalColor;
    glm::vec4 hoverColor;
    glm::vec4 activeColor;
    std::function<void()> onClick;  // callback when clicked

    bool isHovered = false;
    bool isPressed = false;

    // Check if a point is inside this button
    bool contains(float px, float py) const
    {
        return px >= x && px <= (x + width) &&
               py >= y && py <= (y + height);
    }
};

// Main menu UI - renders before the game world loads
class MainMenu
{
public:
    MainMenu(int screenWidth, int screenHeight);
    ~MainMenu();

    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig* phongLight);

    // Input handling
    void handleMouseMove(double mouseX, double mouseY);
    void handleMouseClick(double mouseX, double mouseY, bool pressed);
    
    // Button callbacks - set these to control what happens
    std::function<void()> onPlayClicked;
    std::function<void()> onSettingsClicked;
    std::function<void()> onQuitClicked;

    void updateScreenSize(int width, int height);
    
    // Set skybox for background rendering
    void setSkybox(Skybox* skybox, Shader* skyboxShader) {
        m_skybox = skybox;
        m_skyboxShader = skyboxShader;
    }

private:
    int m_screenWidth;
    int m_screenHeight;

    std::shared_ptr<Shader> m_shader;
    std::unique_ptr<VertexArray> m_quadVAO;
    std::unique_ptr<VertexBuffer> m_quadVBO;
    std::unique_ptr<TextRenderer> m_textRenderer;
    
    Skybox* m_skybox = nullptr;
    Shader* m_skyboxShader = nullptr;

    std::vector<MenuButton> m_buttons;

    // Drawing helpers
    void DrawRect(float x, float y, float width, float height, const glm::vec4& color, float zOffset = 0.0f);
    void DrawButton(const MenuButton& button);
    void DrawText(const std::string& text, float x, float y, float scale, const glm::vec4& color);
    void SetupQuadRendering();
    void InitializeButtons();
};

} // namespace ui
