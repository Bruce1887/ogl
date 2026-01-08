#pragma once

#include "UIConfig.h"
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>

// Forward declarations
class Shader;
class VertexArray;
class VertexBuffer;
class Skybox;
class TextRenderer;
struct PhongLightConfig;

namespace ui
{

/**
 * @brief Loading screen displayed while the game world is being generated
 * 
 * Shows an animated loading indicator and status text while assets
 * and terrain are loaded in the background.
 */
class LoadingScreen
{
public:
    LoadingScreen(int screenWidth, int screenHeight);
    ~LoadingScreen();

    // Update animation (call each frame)
    void update(float deltaTime);

    // Render the loading screen
    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig* phongLight);

    // Screen size updates
    void updateScreenSize(int width, int height);

    // Set loading status text
    void setStatusText(const std::string& text) { m_statusText = text; }

    // Set skybox for background rendering
    void setSkybox(Skybox* skybox) { m_skybox = skybox; }

private:
    int m_screenWidth;
    int m_screenHeight;

    std::shared_ptr<Shader> m_shader;
    std::unique_ptr<VertexArray> m_quadVAO;
    std::unique_ptr<VertexBuffer> m_quadVBO;
    std::unique_ptr<TextRenderer> m_textRenderer;

    Skybox* m_skybox = nullptr;

    // Animation state
    float m_animationTime = 0.0f;
    float m_dotTimer = 0.0f;
    int m_dotCount = 0;
    std::string m_statusText = "Loading ...";
    std::string m_currentTip;  // Random tip selected at construction

    // Drawing helpers
    void DrawRect(float x, float y, float width, float height, const glm::vec4& color, float zOffset = 0.0f);
    void DrawText(const std::string& text, float centerX, float centerY, float scale, const glm::vec4& color);
    void DrawLoadingBar(float x, float y, float width, float height);
    void SetupQuadRendering();
    void SelectRandomTip();

    // Loading tips
    static const std::vector<std::string> s_loadingTips;
};

} // namespace ui
