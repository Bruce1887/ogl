#include "LoadingScreen.h"
#include "../glad/glad.h"
#include "../Common.h"

#include "Shader.h"
#include "Skybox.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "TextRenderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>
#include <random>

namespace ui
{

// Static array of loading tips - add your own game tips here!
const std::vector<std::string> LoadingScreen::s_loadingTips = {
    "Tip: Monster Manges attacks deal a lot of damage, so WATCH OUT!!!",
    "Tip: Use your special abilitiy on 'J' to annihilate enemies.",
    "Tip: Watch out for Abbe the Knight, he is fast!",
};

void LoadingScreen::SelectRandomTip()
{
    if (s_loadingTips.empty()) {
        m_currentTip = "";
        return;
    }
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, s_loadingTips.size() - 1);
    m_currentTip = s_loadingTips[dist(gen)];
}

LoadingScreen::LoadingScreen(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth),
      m_screenHeight(screenHeight)
{
    // Select a random tip for this loading session
    SelectRandomTip();
    
    // Create shader for UI rendering
    try
    {
        m_shader = std::make_shared<Shader>();
        if (!m_shader)
        {
            DEBUG_PRINT("Failed to create loading screen shader");
            return;
        }
        m_shader->addShader("2D.vert", ShaderType::VERTEX);
        m_shader->addShader("uniformColor.frag", ShaderType::FRAGMENT);
        m_shader->createProgram();
        DEBUG_PRINT("LoadingScreen shader created successfully");
    }
    catch (const std::exception& e)
    {
        DEBUG_PRINT("Exception creating loading screen shader: " << e.what());
        m_shader = nullptr;
    }

    try
    {
        SetupQuadRendering();
        DEBUG_PRINT("LoadingScreen quad rendering set up successfully");
    }
    catch (const std::exception& e)
    {
        DEBUG_PRINT("Exception in SetupQuadRendering: " << e.what());
    }

    try
    {
        m_textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight);
        m_textRenderer->LoadFont((FONTS_DIR / "DejaVuSans.ttf").string().c_str(), MENU_FONT_SIZE);
        DEBUG_PRINT("LoadingScreen text renderer initialized successfully");
    }
    catch (const std::exception& e)
    {
        DEBUG_PRINT("Exception initializing TextRenderer: " << e.what());
        m_textRenderer = nullptr;
    }
}

LoadingScreen::~LoadingScreen() = default;

void LoadingScreen::update(float deltaTime)
{
    // Update animation time for spinner
    m_animationTime += deltaTime;
    DEBUG_PRINT("anim time: " << m_animationTime);

    // Animate the dots (cycle every 0.5 seconds)
    m_dotTimer += deltaTime;
    if (m_dotTimer >= 0.5f)
    {
        m_dotTimer = 0.0f;
        m_dotCount = (m_dotCount + 1) % 4; // 0, 1, 2, 3 dots
    }
}

void LoadingScreen::render(glm::mat4 view, glm::mat4 projection, PhongLightConfig* phongLight)
{
    // Render skybox as background if available
    if (m_skybox)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        m_skybox->render(view, projection, nullptr);
    }

    // Disable 3D rendering features for 2D UI
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set up orthographic projection for UI
    glm::mat4 ortho = glm::ortho(
        0.0f, (float)m_screenWidth,
        (float)m_screenHeight, 0.0f,
        -1.0f, 1.0f);

    if (!m_shader) return;
    m_shader->bind();
    m_shader->setUniform("u_Projection", ortho);

    float sw = (float)m_screenWidth;
    float sh = (float)m_screenHeight;
    float scale = UIConfig::scaleUniform(sw, sh);

    // Draw semi-transparent background overlay
    DrawRect(0.0f, 0.0f, sw, sh, {0.02f, 0.02f, 0.05f, 0.85f}, -0.5f);

    // Draw loading panel
    float panelWidth = UIConfig::panelWidthMedium(sw, sh);
    float panelHeight = UIConfig::panelHeightSmall(sw, sh);
    float panelX = UIConfig::centerX(panelWidth, sw);
    float panelY = UIConfig::centerY(panelHeight, sh);


    // Draw "Loading" text with animated dots
    std::string displayText = m_statusText;
    for (int i = 0; i < m_dotCount; ++i)
    {
        displayText += ".";
    }
    
    float textY = panelY + panelHeight * 0.5f;
    float textScale = 1.2f * scale;
    DrawText(displayText, sw * 0.5f, textY, textScale, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    // Draw random tip below the loading text
    float tipY = panelY + panelHeight * 0.88f;
    float tipScale = 0.55f * scale;
    DrawText(m_currentTip, sw * 0.5f, tipY, tipScale, glm::vec4(0.8f, 0.8f, 0.6f, 1.0f));

    // Restore GL state
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void LoadingScreen::updateScreenSize(int width, int height)
{
    m_screenWidth = width;
    m_screenHeight = height;

    if (m_textRenderer)
    {
        m_textRenderer->UpdateScreenSize(width, height);
    }
}

void LoadingScreen::DrawRect(float x, float y, float width, float height, const glm::vec4& color, float zOffset)
{
    if (!m_shader || !m_quadVAO) return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, zOffset));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));

    m_shader->bind();
    m_shader->setUniform("u_Model", model);
    m_shader->setUniform("u_color", color);

    m_quadVAO->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void LoadingScreen::DrawText(const std::string& text, float centerX, float centerY, float scale, const glm::vec4& color)
{
    if (!m_textRenderer) return;
    float textWidth = m_textRenderer->GetTextWidth(text, scale);
    float x = centerX - textWidth * 0.5f;
    float y = centerY + 14.0f * scale;
    m_textRenderer->RenderText(text, x, y, scale, glm::vec3(color));
}

void LoadingScreen::SetupQuadRendering()
{
    float vertices[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    m_quadVAO = std::make_unique<VertexArray>();
    m_quadVBO = std::make_unique<VertexBuffer>(vertices, sizeof(vertices), m_quadVAO.get());

    VertexBufferLayout layout;
    layout.push<float>(2);
    m_quadVAO->addBuffer(m_quadVBO.get(), layout);
}
}