#include "DeathScreen.h"
#include "../glad/glad.h"
#include "../Common.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "TextRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace ui
{

DeathScreen::DeathScreen(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth),
      m_screenHeight(screenHeight)
{
    try
    {
        m_shader = std::make_shared<Shader>();
        m_shader->addShader("2D.vert", ShaderType::VERTEX);
        m_shader->addShader("uniformColor.frag", ShaderType::FRAGMENT);
        m_shader->createProgram();
    }
    catch (const std::exception& e)
    {
        DEBUG_PRINT("Exception creating death screen shader: " << e.what());
        m_shader = nullptr;
    }

    try
    {
        SetupQuadRendering();
    }
    catch (const std::exception& e)
    {
        DEBUG_PRINT("Exception in SetupQuadRendering: " << e.what());
    }

    try
    {
        m_textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight);
        m_textRenderer->LoadFont((FONTS_DIR / "DejaVuSans.ttf").string(), 48);
    }
    catch (const std::exception& e)
    {
        DEBUG_PRINT("Exception initializing TextRenderer: " << e.what());
        m_textRenderer = nullptr;
    }

    // Setup text input box dimensions
    m_inputBoxWidth = 350.0f;
    m_inputBoxHeight = 50.0f;
    m_inputBoxX = (screenWidth - m_inputBoxWidth) * 0.5f;
    m_inputBoxY = screenHeight * 0.48f;

    // Setup submit button dimensions
    m_submitButtonWidth = 200.0f;
    m_submitButtonHeight = 50.0f;
    m_submitButtonX = (screenWidth - m_submitButtonWidth) * 0.5f;
    m_submitButtonY = screenHeight * 0.58f;

    // Setup continue/skip button dimensions
    m_buttonWidth = 200.0f;
    m_buttonHeight = 40.0f;
    m_buttonX = (screenWidth - m_buttonWidth) * 0.5f;
    m_buttonY = screenHeight * 0.68f;
}

DeathScreen::~DeathScreen() = default;

void DeathScreen::SetupQuadRendering()
{
    float vertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    m_quadVAO = std::make_unique<VertexArray>();
    m_quadVBO = std::make_unique<VertexBuffer>(vertices, sizeof(vertices), m_quadVAO.get());

    VertexBufferLayout layout;
    layout.push<float>(3);
    m_quadVAO->addBuffer(m_quadVBO.get(), layout);
}

void DeathScreen::update(float deltaTime)
{
    // Fade in effect
    if (m_fadeAlpha < 1.0f)
    {
        m_fadeAlpha += deltaTime * 2.0f; // Fade in over 0.5 seconds
        if (m_fadeAlpha > 1.0f)
            m_fadeAlpha = 1.0f;
    }
    m_displayTime += deltaTime;

    // Cursor blink timer
    m_cursorBlinkTimer += deltaTime;
    if (m_cursorBlinkTimer >= 0.5f)
    {
        m_cursorBlinkTimer = 0.0f;
        m_showCursor = !m_showCursor;
    }
}

void DeathScreen::render(glm::mat4 view, glm::mat4 projection, PhongLightConfig* phongLight)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 ortho = glm::ortho(
        0.0f, (float)m_screenWidth,
        (float)m_screenHeight, 0.0f,
        -1.0f, 1.0f);

    if (!m_shader)
        return;

    m_shader->bind();
    m_shader->setUniform("u_Projection", ortho);

    // Dark red overlay
    DrawRect(0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight,
             {0.2f, 0.0f, 0.0f, 0.7f * m_fadeAlpha}, -0.5f);

    // "YOU DIED" text
    DrawText("YOU DIED",
             m_screenWidth * 0.5f,
             m_screenHeight * 0.2f,
             3.0f,
             glm::vec4(0.8f, 0.1f, 0.1f, m_fadeAlpha));

    // Display score
    std::string scoreText = "Score: " + std::to_string(m_score);
    DrawText(scoreText,
             m_screenWidth * 0.5f,
             m_screenHeight * 0.32f,
             1.5f,
             glm::vec4(1.0f, 0.9f, 0.7f, m_fadeAlpha));

    // "Enter your name" label
    DrawText("Enter your name:",
             m_screenWidth * 0.5f,
             m_screenHeight * 0.42f,
             0.8f,
             glm::vec4(1.0f, 1.0f, 1.0f, m_fadeAlpha));

    // Text input box background
    glm::vec4 inputBoxColor = m_textInputFocused 
        ? glm::vec4(0.15f, 0.15f, 0.2f, 0.95f * m_fadeAlpha)
        : glm::vec4(0.1f, 0.1f, 0.15f, 0.9f * m_fadeAlpha);
    DrawRect(m_inputBoxX, m_inputBoxY, m_inputBoxWidth, m_inputBoxHeight, inputBoxColor, -0.2f);

    // Text input box border
    float borderThickness = 2.0f;
    glm::vec4 borderColor = m_textInputFocused
        ? glm::vec4(0.6f, 0.6f, 0.8f, m_fadeAlpha)
        : glm::vec4(0.4f, 0.4f, 0.5f, m_fadeAlpha);
    DrawRect(m_inputBoxX - borderThickness, m_inputBoxY - borderThickness, 
             m_inputBoxWidth + 2*borderThickness, borderThickness, borderColor, -0.15f); // top
    DrawRect(m_inputBoxX - borderThickness, m_inputBoxY + m_inputBoxHeight, 
             m_inputBoxWidth + 2*borderThickness, borderThickness, borderColor, -0.15f); // bottom
    DrawRect(m_inputBoxX - borderThickness, m_inputBoxY, 
             borderThickness, m_inputBoxHeight, borderColor, -0.15f); // left
    DrawRect(m_inputBoxX + m_inputBoxWidth, m_inputBoxY, 
             borderThickness, m_inputBoxHeight, borderColor, -0.15f); // right

    // Player name text with cursor
    std::string displayText = m_playerName;
    if (m_textInputFocused && m_showCursor)
    {
        displayText += "|";
    }
    DrawTextLeft(displayText,
             m_inputBoxX + 10.0f,
             m_inputBoxY + m_inputBoxHeight * 0.5f + 8.0f,
             0.8f,
             glm::vec4(1.0f, 1.0f, 1.0f, m_fadeAlpha));

    // Submit button background
    glm::vec4 submitButtonColor = m_submitButtonHovered 
        ? glm::vec4(0.2f, 0.5f, 0.2f, 0.95f * m_fadeAlpha)
        : glm::vec4(0.15f, 0.4f, 0.15f, 0.9f * m_fadeAlpha);
    
    if (m_submitButtonPressed)
        submitButtonColor = glm::vec4(0.25f, 0.6f, 0.25f, 1.0f * m_fadeAlpha);

    DrawRect(m_submitButtonX, m_submitButtonY, m_submitButtonWidth, m_submitButtonHeight, submitButtonColor, -0.2f);

    // Submit button text
    DrawText("Submit Score",
             m_submitButtonX + m_submitButtonWidth * 0.5f,
             m_submitButtonY + m_submitButtonHeight * 0.5f + 8.0f,
             0.9f,
             glm::vec4(1.0f, 1.0f, 1.0f, m_fadeAlpha));

    // Continue/Skip button background
    glm::vec4 buttonColor = m_buttonHovered 
        ? glm::vec4(0.4f, 0.15f, 0.15f, 0.9f * m_fadeAlpha)
        : glm::vec4(0.3f, 0.1f, 0.1f, 0.9f * m_fadeAlpha);
    
    if (m_buttonPressed)
        buttonColor = glm::vec4(0.5f, 0.2f, 0.2f, 1.0f * m_fadeAlpha);

    DrawRect(m_buttonX, m_buttonY, m_buttonWidth, m_buttonHeight, buttonColor, -0.2f);

    // Continue button text
    DrawText("Skip",
             m_buttonX + m_buttonWidth * 0.5f,
             m_buttonY + m_buttonHeight * 0.5f + 6.0f,
             0.7f,
             glm::vec4(0.8f, 0.8f, 0.8f, m_fadeAlpha));

    // Re-enable depth for subsequent rendering
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void DeathScreen::DrawRect(float x, float y, float width, float height, const glm::vec4& color, float zOffset)
{
    if (!m_shader || !m_quadVAO)
        return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, zOffset));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));

    m_shader->setUniform("u_Model", model);
    m_shader->setUniform("u_Color", color);

    m_quadVAO->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void DeathScreen::DrawText(const std::string& text, float x, float y, float scale, const glm::vec4& color)
{
    if (!m_textRenderer)
        return;

    // Calculate text width for centering
    float textWidth = m_textRenderer->GetTextWidth(text, scale);
    float centeredX = x - textWidth * 0.5f;

    m_textRenderer->RenderText(text, centeredX, y, scale, glm::vec3(color.r, color.g, color.b));
}

void DeathScreen::DrawTextLeft(const std::string& text, float x, float y, float scale, const glm::vec4& color)
{
    if (!m_textRenderer)
        return;

    m_textRenderer->RenderText(text, x, y, scale, glm::vec3(color.r, color.g, color.b));
}

void DeathScreen::handleMouseMove(double mouseX, double mouseY)
{
    // Check submit button hover
    m_submitButtonHovered = (mouseX >= m_submitButtonX && mouseX <= m_submitButtonX + m_submitButtonWidth &&
                             mouseY >= m_submitButtonY && mouseY <= m_submitButtonY + m_submitButtonHeight);

    // Check skip/continue button hover
    m_buttonHovered = (mouseX >= m_buttonX && mouseX <= m_buttonX + m_buttonWidth &&
                       mouseY >= m_buttonY && mouseY <= m_buttonY + m_buttonHeight);
}

void DeathScreen::handleMouseClick(double mouseX, double mouseY, bool pressed)
{
    // Check if clicking on text input box
    bool clickedInput = (mouseX >= m_inputBoxX && mouseX <= m_inputBoxX + m_inputBoxWidth &&
                         mouseY >= m_inputBoxY && mouseY <= m_inputBoxY + m_inputBoxHeight);
    
    if (pressed && clickedInput)
    {
        m_textInputFocused = true;
    }
    else if (pressed && !clickedInput && !m_submitButtonHovered && !m_buttonHovered)
    {
        m_textInputFocused = false;
    }

    // Handle submit button
    if (m_submitButtonHovered)
    {
        m_submitButtonPressed = pressed;
        if (!pressed && onSubmitScore)
        {
            onSubmitScore(m_playerName, m_score);
        }
    }
    else
    {
        m_submitButtonPressed = false;
    }

    // Handle skip/continue button
    if (m_buttonHovered)
    {
        m_buttonPressed = pressed;
        if (!pressed && onContinueClicked)
        {
            onContinueClicked();
        }
    }
    else
    {
        m_buttonPressed = false;
    }
}

void DeathScreen::handleCharInput(unsigned int codepoint)
{
    if (!m_textInputFocused)
        return;

    // Only accept printable ASCII characters
    if (codepoint >= 32 && codepoint < 127 && m_playerName.length() < MAX_NAME_LENGTH)
    {
        m_playerName += static_cast<char>(codepoint);
        m_showCursor = true;
        m_cursorBlinkTimer = 0.0f;
    }
}

void DeathScreen::handleKeyInput(int key, int action)
{
    if (!m_textInputFocused || action != 1) // 1 = GLFW_PRESS
        return;

    if (key == 259 && !m_playerName.empty()) // 259 = GLFW_KEY_BACKSPACE
    {
        m_playerName.pop_back();
        m_showCursor = true;
        m_cursorBlinkTimer = 0.0f;
    }
    else if (key == 257 && onSubmitScore) // 257 = GLFW_KEY_ENTER
    {
        onSubmitScore(m_playerName, m_score);
    }
}

void DeathScreen::updateScreenSize(int width, int height)
{
    m_screenWidth = width;
    m_screenHeight = height;

    // Update text input box position
    m_inputBoxX = (width - m_inputBoxWidth) * 0.5f;
    m_inputBoxY = height * 0.48f;

    // Update submit button position
    m_submitButtonX = (width - m_submitButtonWidth) * 0.5f;
    m_submitButtonY = height * 0.58f;

    // Update skip button position
    m_buttonX = (width - m_buttonWidth) * 0.5f;
    m_buttonY = height * 0.68f;
}

} // namespace ui
