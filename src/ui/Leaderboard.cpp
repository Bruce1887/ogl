#include "Leaderboard.h"
#include "../glad/glad.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "TextRenderer.h"
#include "Skybox.h"
#include "Common.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <iostream>
#include <algorithm>

namespace ui
{

Leaderboard::Leaderboard(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth),
      m_screenHeight(screenHeight)
{
    try {
        m_shader = std::make_shared<Shader>();
        if (!m_shader) {
            DEBUG_PRINT("Failed to create leaderboard shader");
            return;
        }
        m_shader->addShader("2D.vert", ShaderType::VERTEX);
        m_shader->addShader("uniformColor.frag", ShaderType::FRAGMENT);
        m_shader->createProgram();
        DEBUG_PRINT("Leaderboard shader created successfully");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Exception creating leaderboard shader: " << e.what());
        m_shader = nullptr;
    }

    try {
        SetupQuadRendering();
        DEBUG_PRINT("Leaderboard quad rendering set up successfully");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Exception in SetupQuadRendering: " << e.what());
    }
    
    try {
        m_textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight);
        m_textRenderer->LoadFont((FONTS_DIR / "DejaVuSans.ttf").string().c_str(), 48);
        DEBUG_PRINT("Leaderboard text renderer initialized successfully");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Exception initializing TextRenderer: " << e.what());
        m_textRenderer = nullptr;
    }
    
    try {
        InitializeButtons();
        DEBUG_PRINT("Leaderboard buttons initialized successfully");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Exception in InitializeButtons: " << e.what());
    }
}

Leaderboard::~Leaderboard() = default;

void Leaderboard::render(glm::mat4 view, glm::mat4 projection, PhongLightConfig* phongLight)
{
    // Render skybox background if available (Skybox owns its shader now)
    if (m_skybox) {
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
        -1.0f, 1.0f
    );

    if (!m_shader) return;
    m_shader->bind();
    m_shader->setUniform("u_Projection", ortho);

    // Draw semi-transparent background overlay
    DrawRect(0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, 
             {0.05f, 0.05f, 0.1f, 0.5f}, -0.5f);

    // Draw leaderboard panel
    float panelWidth = 600.0f;
    float panelHeight = 500.0f;
    float panelX = (m_screenWidth - panelWidth) * 0.5f;
    float panelY_original = (m_screenHeight - panelHeight) * 0.5f;
    float panelY = panelY_original - 30.0f;
    DrawRect(panelX, panelY, panelWidth, panelHeight, {0.1f, 0.1f, 0.2f, 0.9f}, -0.2f);

    // Draw title (keep at original position)
    DrawText("LEADERBOARD", 
             m_screenWidth * 0.5f,
             panelY_original + 30.0f,
             1.5f,
             glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    // Column layout: Rank # on far left, player name next, kills and time on right
    float rankX = panelX + 20.0f;
    float nameX = panelX + 70.0f;
    float killsX = panelX + panelWidth - 180.0f;
    float timeX = panelX + panelWidth - 80.0f;
    float entryY = panelY_original + 80.0f;
    float entrySpacing = 35.0f;

    // Draw column headers
    DrawText("#", rankX + 10.0f, entryY, 0.8f, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
    DrawText("Player", nameX + 60.0f, entryY, 0.8f, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
    DrawText("Kills", killsX, entryY, 0.8f, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
    DrawText("Time", timeX, entryY, 0.8f, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));

    // Draw leaderboard entries (top 10)
    entryY += entrySpacing + 10.0f;
    int maxEntries = std::min(10, (int)m_entries.size());
    for (int i = 0; i < maxEntries; ++i)
    {
        const auto& entry = m_entries[i];
        std::string rankStr = std::to_string(entry.rank);
        std::string killsStr = std::to_string(entry.kills);

        // Draw rank number (left-aligned)
        float rankWidth = m_textRenderer ? m_textRenderer->GetTextWidth(rankStr, 0.7f) : 20.0f;
        DrawText(rankStr, 
                rankX + rankWidth * 0.5f,
                entryY, 0.7f, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
                
        // Draw player name (left-aligned)
        float nameWidth = m_textRenderer ? m_textRenderer->GetTextWidth(entry.playerName, 0.7f) : 100.0f;
        DrawText(entry.playerName, 
                nameX + nameWidth * 0.5f,
                entryY, 0.7f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        
        // Draw kills
        DrawText(killsStr, killsX, entryY, 0.7f, glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
        
        // Draw time
        DrawText(entry.time, timeX, entryY, 0.7f, glm::vec4(0.5f, 1.0f, 0.5f, 1.0f));

        entryY += entrySpacing;
    }

    // Draw buttons
    for (const auto& button : m_buttons)
    {
        DrawButton(button);
    }

    // Restore GL state
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void Leaderboard::handleMouseMove(double mouseX, double mouseY)
{
    for (auto& button : m_buttons)
    {
        button.isHovered = button.contains((float)mouseX, (float)mouseY);
    }
}

void Leaderboard::handleMouseClick(double mouseX, double mouseY, bool pressed)
{
    for (auto& button : m_buttons)
    {
        bool contains = button.contains((float)mouseX, (float)mouseY);
        
        if (contains)
        {
            button.isPressed = pressed;
            
            if (!pressed && button.onClick)
            {
                DEBUG_PRINT("Leaderboard button clicked: " << button.label);
                button.onClick();
            }
        }
        else
        {
            button.isPressed = false;
        }
    }
}

void Leaderboard::updateScreenSize(int width, int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
    InitializeButtons();
}

void Leaderboard::addEntry(const std::string& name, int kills, const std::string& time)
{
    LeaderboardEntry entry{name, kills, time, (int)m_entries.size() + 1};
    m_entries.push_back(entry);
    
    // Sort by kills descending
    std::sort(m_entries.begin(), m_entries.end(), 
        [](const LeaderboardEntry& a, const LeaderboardEntry& b) { 
            return a.kills > b.kills; 
        });
    
    // Update ranks
    for (size_t i = 0; i < m_entries.size(); ++i) {
        m_entries[i].rank = i + 1;
    }
}

void Leaderboard::clearEntries()
{
    m_entries.clear();
}

void Leaderboard::DrawRect(float x, float y, float width, float height, const glm::vec4& color, float zOffset)
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

void Leaderboard::DrawButton(const LeaderboardButton& button)
{
    glm::vec4 color = button.normalColor;
    if (button.isPressed)
        color = button.activeColor;
    else if (button.isHovered)
        color = button.hoverColor;

    DrawRect(button.x, button.y, button.width, button.height, color, 0.0f);

    float borderWidth = 3.0f;
    glm::vec4 borderColor = button.isHovered ? 
        glm::vec4(0.6f, 0.6f, 0.8f, 1.0f) : 
        glm::vec4(0.3f, 0.3f, 0.4f, 1.0f);

    DrawRect(button.x, button.y, button.width, borderWidth, borderColor, 0.1f);
    DrawRect(button.x, button.y + button.height - borderWidth, button.width, borderWidth, borderColor, 0.1f);
    DrawRect(button.x, button.y, borderWidth, button.height, borderColor, 0.1f);
    DrawRect(button.x + button.width - borderWidth, button.y, borderWidth, button.height, borderColor, 0.1f);

    DrawText(button.label, 
             button.x + button.width * 0.5f,
             button.y + button.height * 0.5f,
             1.0f, 
             glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
}

void Leaderboard::DrawText(const std::string& text, float centerX, float centerY, float scale, const glm::vec4& color)
{
    if (!m_textRenderer) return;
    float textWidth = m_textRenderer->GetTextWidth(text, scale);
    float x = centerX - textWidth * 0.5f;
    float y = centerY + 14.0f * scale;
    m_textRenderer->RenderText(text, x, y, scale, glm::vec3(color));
}

void Leaderboard::SetupQuadRendering()
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

void Leaderboard::InitializeButtons()
{
    m_buttons.clear();

    float buttonWidth = 200.0f;
    float buttonHeight = 60.0f;
    float panelWidth = 600.0f;
    float panelHeight = 500.0f;
    float panelX = (m_screenWidth - panelWidth) * 0.5f;
    float panelY = (m_screenHeight - panelHeight) * 0.5f;

    // Back button
    LeaderboardButton backBtn;
    backBtn.label = "Back";
    backBtn.x = panelX + (panelWidth - buttonWidth) * 0.5f;
    backBtn.y = panelY + panelHeight - buttonHeight + 75.0f;
    backBtn.width = buttonWidth;
    backBtn.height = buttonHeight;
    backBtn.normalColor = {0.3f, 0.3f, 0.5f, 0.9f};
    backBtn.hoverColor = {0.4f, 0.4f, 0.7f, 1.0f};
    backBtn.activeColor = {0.2f, 0.2f, 0.4f, 1.0f};
    backBtn.onClick = [this]() { 
        if (onBackClicked) onBackClicked(); 
    };
    m_buttons.push_back(backBtn);
}

} // namespace ui
