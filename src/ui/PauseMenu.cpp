#include "PauseMenu.h"
#include "../glad/glad.h"
#include "../Common.h"

#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "TextRenderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <iostream>

namespace ui
{

    PauseMenu::PauseMenu(int screenWidth, int screenHeight)
        : m_screenWidth(screenWidth),
          m_screenHeight(screenHeight)
    {
        try
        {
            m_shader = std::make_shared<Shader>();
            if (!m_shader)
            {
                DEBUG_PRINT("Failed to create pause menu shader");
                return;
            }
            m_shader->addShader("2D.vert", ShaderType::VERTEX);
            m_shader->addShader("uniformColor.frag", ShaderType::FRAGMENT);
            m_shader->createProgram();
        }
        catch (const std::exception &e)
        {
            DEBUG_PRINT("Exception creating pause menu shader: " << e.what());
            m_shader = nullptr;
        }

        try
        {
            SetupQuadRendering();
        }
        catch (const std::exception &e)
        {
            DEBUG_PRINT("Exception in SetupQuadRendering: " << e.what());
        }

        try
        {
            m_textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight);
            m_textRenderer->LoadFont((FONTS_DIR / "DejaVuSans.ttf").string().c_str(), MENU_FONT_SIZE);
            DEBUG_PRINT("PauseMenu text renderer initialized successfully");
        }
        catch (const std::exception &e)
        {
            DEBUG_PRINT("Exception initializing TextRenderer: " << e.what());
            m_textRenderer = nullptr;
        }

        try
        {
            InitializeButtons();
        }
        catch (const std::exception &e)
        {
            DEBUG_PRINT("Exception in InitializeButtons: " << e.what());
        }
    }

    PauseMenu::~PauseMenu() = default;

    void PauseMenu::render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight)
    {
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

        if (!m_shader)
            return;
        m_shader->bind();
        m_shader->setUniform("u_Projection", ortho);

        // Draw grayish semi-transparent background overlay
        DrawRect(0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight,
                 {0.3f, 0.3f, 0.3f, 0.6f}, -0.5f);

        // Draw pause menu panel (darker background)
        float panelWidth = 500.0f;
        float panelHeight = 350.0f;
        float panelX = (m_screenWidth - panelWidth) * 0.5f;
        float panelY = (m_screenHeight - panelHeight) * 0.5f;
        DrawRect(panelX, panelY, panelWidth, panelHeight, {0.1f, 0.1f, 0.15f, 0.95f}, -0.2f);

        // Draw title
        DrawText("PAUSED",
                 m_screenWidth * 0.5f,
                 panelY + 50.0f,
                 1.5f,
                 glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Draw all buttons
        for (const auto &button : m_buttons)
        {
            DrawButton(button);
        }

        // Restore GL state
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }

    void PauseMenu::handleMouseMove(double mouseX, double mouseY)
    {
        for (auto &button : m_buttons)
        {
            button.isHovered = button.contains((float)mouseX, (float)mouseY);
        }
    }

    void PauseMenu::handleMouseClick(double mouseX, double mouseY, bool pressed)
    {
        for (auto &button : m_buttons)
        {
            bool contains = button.contains((float)mouseX, (float)mouseY);

            if (contains)
            {
                button.isPressed = pressed;

                if (!pressed && button.onClick)
                {
                    DEBUG_PRINT("Pause menu button clicked: " << button.label);
                    button.onClick();
                }
            }
            else
            {
                button.isPressed = false;
            }
        }
    }

    void PauseMenu::updateScreenSize(int width, int height)
    {
        m_screenWidth = width;
        m_screenHeight = height;
        InitializeButtons();
    }

    void PauseMenu::DrawRect(float x, float y, float width, float height, const glm::vec4 &color, float zOffset)
    {
        if (!m_shader || !m_quadVAO)
            return;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, zOffset));
        model = glm::scale(model, glm::vec3(width, height, 1.0f));

        m_shader->bind();
        m_shader->setUniform("u_Model", model);
        m_shader->setUniform("u_color", color);

        m_quadVAO->bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void PauseMenu::DrawButton(const PauseButton &button)
    {
        glm::vec4 color = button.normalColor;
        if (button.isPressed)
            color = button.activeColor;
        else if (button.isHovered)
            color = button.hoverColor;

        DrawRect(button.x, button.y, button.width, button.height, color, 0.0f);

        float borderWidth = 3.0f;
        glm::vec4 borderColor = button.isHovered ? glm::vec4(0.6f, 0.6f, 0.8f, 1.0f) : glm::vec4(0.3f, 0.3f, 0.4f, 1.0f);

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

    void PauseMenu::DrawText(const std::string &text, float centerX, float centerY, float scale, const glm::vec4 &color)
    {
        if (!m_textRenderer)
            return;

        float textWidth = m_textRenderer->GetTextWidth(text, scale);

        // Center horizontally
        float x = centerX - textWidth * 0.5f;
        // For vertical centering, use centerY directly as baseline and nudge downward
        float y = centerY + 14.0f * scale; // Slight downward offset for better centering

        m_textRenderer->RenderText(text, x, y, scale, glm::vec3(color));
    }

    void PauseMenu::SetupQuadRendering()
    {
        float vertices[] = {
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 1.0f};

        m_quadVAO = std::make_unique<VertexArray>();
        m_quadVBO = std::make_unique<VertexBuffer>(vertices, sizeof(vertices), m_quadVAO.get());

        VertexBufferLayout layout;
        layout.push<float>(2);
        m_quadVAO->addBuffer(m_quadVBO.get(), layout);
    }

    void PauseMenu::InitializeButtons()
    {
        m_buttons.clear();

        float buttonWidth = 350.0f;
        float buttonHeight = 70.0f;
        float buttonSpacing = 20.0f;

        float startX = (m_screenWidth - buttonWidth) * 0.5f;
        float startY = (m_screenHeight - 300.0f) * 0.5f + 65.0f;

        // Resume button
        PauseButton resumeBtn;
        resumeBtn.label = "Resume";
        resumeBtn.x = startX;
        resumeBtn.y = startY;
        resumeBtn.width = buttonWidth;
        resumeBtn.height = buttonHeight;
        resumeBtn.normalColor = {0.2f, 0.5f, 0.2f, 0.9f};
        resumeBtn.hoverColor = {0.3f, 0.7f, 0.3f, 1.0f};
        resumeBtn.activeColor = {0.1f, 0.4f, 0.1f, 1.0f};
        resumeBtn.onClick = [this]()
        {
            if (onResumeClicked)
                onResumeClicked();
        };
        m_buttons.push_back(resumeBtn);

        // Settings button
        PauseButton settingsBtn;
        settingsBtn.label = "Settings";
        settingsBtn.x = startX;
        settingsBtn.y = startY + buttonHeight + buttonSpacing;
        settingsBtn.width = buttonWidth;
        settingsBtn.height = buttonHeight;
        settingsBtn.normalColor = {0.3f, 0.3f, 0.5f, 0.9f};
        settingsBtn.hoverColor = {0.4f, 0.4f, 0.7f, 1.0f};
        settingsBtn.activeColor = {0.2f, 0.2f, 0.4f, 1.0f};
        settingsBtn.onClick = [this]()
        {
            if (onSettingsClicked)
                onSettingsClicked();
        };
        m_buttons.push_back(settingsBtn);

        // Quit button
        PauseButton quitBtn;
        quitBtn.label = "Quit to Menu";
        quitBtn.x = startX;
        quitBtn.y = startY + 2 * (buttonHeight + buttonSpacing);
        quitBtn.width = buttonWidth;
        quitBtn.height = buttonHeight;
        quitBtn.normalColor = {0.5f, 0.2f, 0.2f, 0.9f};
        quitBtn.hoverColor = {0.7f, 0.3f, 0.3f, 1.0f};
        quitBtn.activeColor = {0.4f, 0.1f, 0.1f, 1.0f};
        quitBtn.onClick = [this]()
        {
            if (onQuitClicked)
                onQuitClicked();
        };
        m_buttons.push_back(quitBtn);
    }

} // namespace ui
