#include "HUDEntityImpl.h"

#include "../glad/glad.h"
#include "../Common.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "MeshRenderable.h"
#include "TextRenderer.h"

#include <memory>
#include <string>
#include <sstream>
#include <iomanip>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

HUDEntityImpl::HUDEntityImpl(Player* player, int screenWidth, int screenHeight)
    : m_player(player),
      m_screenWidth(screenWidth),
      m_screenHeight(screenHeight)
{
    auto tempShader = std::make_shared<Shader>();
    tempShader->addShader("2D.vert", ShaderType::VERTEX); 
    tempShader->addShader("uniformColor.frag", ShaderType::FRAGMENT);
    tempShader->createProgram();
    m_shaderRef = tempShader;
    m_shaderRef->bind();

    SetupQuadRendering();
    
    // Initialize text renderer
    try {
        m_textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight);
        m_textRenderer->LoadFont((FONTS_DIR / "DejaVuSans.ttf").string().c_str(), MENU_FONT_SIZE);
    } catch (const std::exception& e) {
        DEBUG_PRINT("Failed to initialize HUD text renderer: " << e.what());
        m_textRenderer = nullptr;
    }
}

HUDEntityImpl::~HUDEntityImpl() = default; // Smart pointers handle cleanup

void HUDEntityImpl::updateScreenSize(int width, int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
    
    if (m_textRenderer) {
        m_textRenderer->UpdateScreenSize(width, height);
    }
}

void HUDEntityImpl::render(const glm::mat4 view, const glm::mat4 projection, const PhongLightConfig *phongLight) {
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE); // Disable depth writing
    glDisable(GL_CULL_FACE); // Disable face culling for HUD
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glm::mat4 ortho = glm::ortho(
        0.0f, (float)m_screenWidth,
        (float)m_screenHeight, 0.0f,
        -1.0f, 1.0f  // near/far planes for depth layering
    );

    m_shaderRef->bind();
    m_shaderRef->setUniform("u_Projection", ortho);

    DrawVitalsBars();
    DrawTimer();
    DrawScoreAndWave();

    glDisable(GL_BLEND);  // Disable blending after HUD
    glDepthMask(GL_TRUE); // Re-enable depth writing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void HUDEntityImpl::DrawVitalsBars() {
    float health = m_player ? m_player->m_playerData.m_health : 0.0f;
    float maxHealth = m_player ? m_player->m_playerData.m_maxHealth : 100.0f;

    float sw = (float)m_screenWidth;
    float sh = (float)m_screenHeight;

    // Use scaled UIConfig for consistent HUD positioning at any resolution
    float x_start = UIConfig::hudMargin(sw, sh);
    float y_start = UIConfig::hudMargin(sw, sh);
    float bar_width = UIConfig::hudBarWidth(sw, sh);
    float bar_height = UIConfig::hudBarHeight(sw, sh);

    // Health Bar (background at z=-0.1, foreground at z=0.0 to layer properly)
    DrawRect(x_start, y_start, bar_width, bar_height, {0.3f, 0.0f, 0.0f, 1.0f}, -0.1f);
    DrawRect(x_start, y_start, bar_width * (health / maxHealth), bar_height, {0.8f, 0.1f, 0.1f, 1.0f}, 0.0f);
}

void HUDEntityImpl::DrawTimer() {
    if (!m_textRenderer || !m_gameClock) return;
    
    float sw = (float)m_screenWidth;
    float sh = (float)m_screenHeight;
    float scale = UIConfig::scaleUniform(sw, sh);
    
    // Get elapsed time
    int totalSeconds = static_cast<int>(m_gameClock->GetTotalElapsedSeconds());
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    
    // Format as MM:SS or just SS if under a minute
    std::ostringstream oss;
    if (minutes > 0) {
        oss << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
    } else {
        oss << seconds;
    }
    std::string timeStr = oss.str();
    
    // Draw centered at top
    float textScale = 1.2f * scale;
    float textWidth = m_textRenderer->GetTextWidth(timeStr, textScale);
    float x = (sw - textWidth) * 0.5f;
    float y = UIConfig::hudMargin(sw, sh) + 20.0f * scale;
    
    // Draw with a slight shadow for visibility
    m_textRenderer->RenderText(timeStr, x + 2*scale, y + 2*scale, textScale, glm::vec3(0.0f, 0.0f, 0.0f));
    m_textRenderer->RenderText(timeStr, x, y, textScale, glm::vec3(1.0f, 1.0f, 1.0f));
}

void HUDEntityImpl::DrawScoreAndWave() {
    if (!m_textRenderer || !m_player) return;
    
    float sw = (float)m_screenWidth;
    float sh = (float)m_screenHeight;
    float scale = UIConfig::scaleUniform(sw, sh);
    float margin = UIConfig::hudMargin(sw, sh);
    
    float textScale = 0.8f * scale;
    float lineHeight = 35.0f * scale;
    
    // Get score from player
    int score = m_player->getScore();
    
    // Wave text
    std::string waveStr = "Wave: " + std::to_string(m_currentWave);
    float waveWidth = m_textRenderer->GetTextWidth(waveStr, textScale);
    float waveX = sw - margin - waveWidth;
    float waveY = margin + 20.0f * scale;
    
    // Score text
    std::string scoreStr = "Score: " + std::to_string(score);
    float scoreWidth = m_textRenderer->GetTextWidth(scoreStr, textScale);
    float scoreX = sw - margin - scoreWidth;
    float scoreY = waveY + lineHeight;
    
    // Draw wave with shadow
    m_textRenderer->RenderText(waveStr, waveX + 2*scale, waveY + 2*scale, textScale, glm::vec3(0.0f, 0.0f, 0.0f));
    m_textRenderer->RenderText(waveStr, waveX, waveY, textScale, glm::vec3(1.0f, 0.9f, 0.3f));  // Yellow
    
    // Draw score with shadow
    m_textRenderer->RenderText(scoreStr, scoreX + 2*scale, scoreY + 2*scale, textScale, glm::vec3(0.0f, 0.0f, 0.0f));
    m_textRenderer->RenderText(scoreStr, scoreX, scoreY, textScale, glm::vec3(1.0f, 0.4f, 0.4f));  // Red
}

void HUDEntityImpl::DrawText(const std::string& text, float x, float y, float scale, const glm::vec3& color) {
    if (m_textRenderer) {
        m_textRenderer->RenderText(text, x, y, scale, color);
    }
}

void HUDEntityImpl::DrawRect(float x, float y, float width, float height, const glm::vec4& color, float zOffset) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, zOffset));  // use zOffset to layer UI elements
    model = glm::scale(model, glm::vec3(width, height, 1.0f));

    m_shaderRef->bind();
    m_shaderRef->setUniform("u_Model", model);
    m_shaderRef->setUniform("u_color", color);

    m_quadVAO->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// -------------------- QUAD SETUP --------------------
void HUDEntityImpl::SetupQuadRendering() {
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
