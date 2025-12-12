#include "MainMenu.h"
#include "../glad/glad.h"
#include "Shader.h"
#include "Skybox.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "TextRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <iostream>

namespace ui
{

MainMenu::MainMenu(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth),
      m_screenHeight(screenHeight)
{
    // Create shader for UI rendering
    try {
        m_shader = std::make_shared<Shader>();
        if (!m_shader) {
            std::cerr << "Failed to create shader" << std::endl;
            return;
        }
        m_shader->addShader("2D.vert", ShaderType::VERTEX);
        m_shader->addShader("uniformColor.frag", ShaderType::FRAGMENT);
        m_shader->createProgram();
        std::cout << "MainMenu shader created successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception creating shader: " << e.what() << std::endl;
        m_shader = nullptr;
    }

    try {
        SetupQuadRendering();
        std::cout << "MainMenu quad rendering set up successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception in SetupQuadRendering: " << e.what() << std::endl;
    }
    
    try {
        m_textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight);
        m_textRenderer->LoadFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 48);
        std::cout << "MainMenu text renderer initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception initializing TextRenderer: " << e.what() << std::endl;
        m_textRenderer = nullptr;
    }
    
    try {
        InitializeButtons();
        std::cout << "MainMenu buttons initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception in InitializeButtons: " << e.what() << std::endl;
    }
}

MainMenu::~MainMenu() = default;

void MainMenu::render(glm::mat4 view, glm::mat4 projection, PhongLightConfig* phongLight)
{
    // First, render skybox as background if available
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

    if (!m_shader) return;  // Safety check
    m_shader->bind();
    m_shader->setUniform("u_Projection", ortho);

    // Draw semi-transparent background overlay (over skybox)
    DrawRect(0.0f, 0.0f, (float)m_screenWidth, (float)m_screenHeight, 
             {0.05f, 0.05f, 0.1f, 0.5f}, -0.5f);

    // Draw title background
    float titleWidth = 600.0f;
    float titleHeight = 120.0f;
    float titleX = (m_screenWidth - titleWidth) * 0.5f;
    float titleY = 80.0f;
    DrawRect(titleX, titleY, titleWidth, titleHeight, {0.1f, 0.1f, 0.2f, 0.9f}, -0.2f);

    // Draw game name
    DrawText("OogaBooga", 
             m_screenWidth * 0.5f,
             titleY + 60.0f,
             2.0f,
             glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    // Draw all buttons
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

void MainMenu::handleMouseMove(double mouseX, double mouseY)
{
    static int debugCounter = 0;
    if (debugCounter++ % 60 == 0) {  // Print every 60 frames to avoid spam
        std::cout << "Mouse position: (" << mouseX << ", " << mouseY << ")" << std::endl;
    }
    
    // Update hover state for all buttons
    for (auto& button : m_buttons)
    {
        bool wasHovered = button.isHovered;
        button.isHovered = button.contains((float)mouseX, (float)mouseY);
        
        if (button.isHovered && !wasHovered) {
            std::cout << "Hovering over: " << button.label << std::endl;
        }
    }
}

void MainMenu::handleMouseClick(double mouseX, double mouseY, bool pressed)
{
    if (pressed) {
        std::cout << "Mouse clicked at (" << mouseX << ", " << mouseY << ")" << std::endl;
    }
    
    for (auto& button : m_buttons)
    {
        bool contains = button.contains((float)mouseX, (float)mouseY);
        
        if (pressed) {
            std::cout << "  Checking button '" << button.label << "' [" 
                      << button.x << "," << button.y << " to " 
                      << (button.x + button.width) << "," << (button.y + button.height) << "]: "
                      << (contains ? "HIT" : "miss") << std::endl;
        }
        
        if (contains)
        {
            button.isPressed = pressed;
            
            // On mouse release (click complete), trigger callback
            if (!pressed && button.onClick)
            {
                std::cout << "Button clicked: " << button.label << std::endl;
                button.onClick();
            }
        }
        else
        {
            button.isPressed = false;
        }
    }
}

void MainMenu::updateScreenSize(int width, int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
    
    // Recenter buttons
    InitializeButtons();
}

void MainMenu::DrawRect(float x, float y, float width, float height, const glm::vec4& color, float zOffset)
{
    if (!m_shader || !m_quadVAO) return;  // Safety check
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, zOffset));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));

    m_shader->bind();
    m_shader->setUniform("u_Model", model);
    m_shader->setUniform("u_color", color);

    m_quadVAO->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void MainMenu::DrawButton(const MenuButton& button)
{
    // Choose color based on button state
    glm::vec4 color = button.normalColor;
    if (button.isPressed)
        color = button.activeColor;
    else if (button.isHovered)
        color = button.hoverColor;

    // Draw button background
    DrawRect(button.x, button.y, button.width, button.height, color, 0.0f);

    // Draw button border/highlight
    float borderWidth = 3.0f;
    glm::vec4 borderColor = button.isHovered ? 
        glm::vec4(0.6f, 0.6f, 0.8f, 1.0f) : 
        glm::vec4(0.3f, 0.3f, 0.4f, 1.0f);

    // Top border
    DrawRect(button.x, button.y, button.width, borderWidth, borderColor, 0.1f);
    // Bottom border
    DrawRect(button.x, button.y + button.height - borderWidth, button.width, borderWidth, borderColor, 0.1f);
    // Left border
    DrawRect(button.x, button.y, borderWidth, button.height, borderColor, 0.1f);
    // Right border
    DrawRect(button.x + button.width - borderWidth, button.y, borderWidth, button.height, borderColor, 0.1f);

    // Draw button text centered in the button
    DrawText(button.label, 
             button.x + button.width * 0.5f,
             button.y + button.height * 0.5f,
             1.0f, 
             glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
}

void MainMenu::DrawText(const std::string& text, float centerX, float centerY,
                        float scale, const glm::vec4& color)
{
    if (!m_textRenderer) return;

    float textWidth = m_textRenderer->GetTextWidth(text, scale);

    // Center horizontally
    float x = centerX - textWidth * 0.5f;
    // For vertical centering, use centerY directly as baseline and nudge downward
    float y = centerY + 17.5f * scale;  // Slight downward offset for better centering

    m_textRenderer->RenderText(text, x, y, scale, glm::vec3(color));
}

void MainMenu::SetupQuadRendering()
{
    // Simple unit quad (0,0) to (1,1)
    float vertices[] = {
        0.0f, 1.0f, 
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    try {
        m_quadVAO = std::make_unique<VertexArray>();
        std::cout << "VertexArray created with ID: " << m_quadVAO->getID() << std::endl;
        
        m_quadVBO = std::make_unique<VertexBuffer>(vertices, sizeof(vertices), m_quadVAO.get());
        std::cout << "VertexBuffer created with size: " << m_quadVBO->getSize() << std::endl;

        VertexBufferLayout layout;
        std::cout << "VertexBufferLayout created, stride before push: " << layout.getStride() << std::endl;
        
        layout.push<float>(2);
        std::cout << "After push, stride: " << layout.getStride() << std::endl;
        
        m_quadVAO->addBuffer(m_quadVBO.get(), layout);
        std::cout << "VertexBufferLayout added successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception in SetupQuadRendering: " << e.what() << std::endl;
        m_quadVAO.reset();
        m_quadVBO.reset();
    }
}

void MainMenu::InitializeButtons()
{
    m_buttons.clear();

    // Button dimensions
    float buttonWidth = 400.0f;
    float buttonHeight = 70.0f;
    float buttonSpacing = 20.0f;
    
    // Center buttons horizontally
    float startX = (m_screenWidth - buttonWidth) * 0.5f;
    // Start buttons below the title area
    float startY = 280.0f;

    std::cout << "Initializing buttons for screen " << m_screenWidth << "x" << m_screenHeight << std::endl;
    std::cout << "Button start position: (" << startX << ", " << startY << ")" << std::endl;

    // Play button
    MenuButton playBtn;
    playBtn.label = "Play Game";
    playBtn.x = startX;
    playBtn.y = startY;
    playBtn.width = buttonWidth;
    playBtn.height = buttonHeight;
    playBtn.normalColor = {0.2f, 0.5f, 0.2f, 0.9f};  // greenish
    playBtn.hoverColor = {0.3f, 0.7f, 0.3f, 1.0f};
    playBtn.activeColor = {0.1f, 0.4f, 0.1f, 1.0f};
    playBtn.onClick = [this]() { 
        if (onPlayClicked) onPlayClicked(); 
    };
    m_buttons.push_back(playBtn);
    std::cout << "Play button: x=" << playBtn.x << " y=" << playBtn.y 
              << " w=" << playBtn.width << " h=" << playBtn.height << std::endl;

    // Leaderboard button
    MenuButton settingsBtn;
    settingsBtn.label = "Leaderboard";
    settingsBtn.x = startX;
    settingsBtn.y = startY + buttonHeight + buttonSpacing;
    settingsBtn.width = buttonWidth;
    settingsBtn.height = buttonHeight;
    settingsBtn.normalColor = {0.3f, 0.3f, 0.5f, 0.9f};  // blueish
    settingsBtn.hoverColor = {0.4f, 0.4f, 0.7f, 1.0f};
    settingsBtn.activeColor = {0.2f, 0.2f, 0.4f, 1.0f};
    settingsBtn.onClick = [this]() { 
        if (onSettingsClicked) onSettingsClicked(); 
    };
    m_buttons.push_back(settingsBtn);

    // Quit button
    MenuButton quitBtn;
    quitBtn.label = "Quit";
    quitBtn.x = startX;
    quitBtn.y = startY + 2 * (buttonHeight + buttonSpacing);
    quitBtn.width = buttonWidth;
    quitBtn.height = buttonHeight;
    quitBtn.normalColor = {0.5f, 0.2f, 0.2f, 0.9f};  // reddish
    quitBtn.hoverColor = {0.7f, 0.3f, 0.3f, 1.0f};
    quitBtn.activeColor = {0.4f, 0.1f, 0.1f, 1.0f};
    quitBtn.onClick = [this]() { 
        if (onQuitClicked) onQuitClicked(); 
    };
    m_buttons.push_back(quitBtn);
}

} // namespace ui
