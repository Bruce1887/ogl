#include "HUDEntityImpl.h"

#include "../glad/glad.h"
#include "../Common.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "MeshRenderable.h"

#include <memory>
#include <string>
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
}

HUDEntityImpl::~HUDEntityImpl() = default; // Smart pointers handle cleanup

void HUDEntityImpl::render(glm::mat4 view, glm::mat4 projection, PhongLightConfig* phongLight) {
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
    // DrawClockDisplay();

    glDisable(GL_BLEND);  // Disable blending after HUD
    glDepthMask(GL_TRUE); // Re-enable depth writing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void HUDEntityImpl::DrawVitalsBars() {
    float health = m_player ? m_player->m_playerData.m_health : 0.0f;
    float maxHealth = m_player ? m_player->m_playerData.m_maxHealth : 100.0f;

    float x_start = 50.0f;
    float y_start = 50.0f;
    float bar_width = 200.0f;
    float bar_height = 20.0f;

    // Health Bar (background at z=-0.1, foreground at z=0.0 to layer properly)
    DrawRect(x_start, y_start, bar_width, bar_height, {0.3f, 0.0f, 0.0f, 1.0f}, -0.1f);
    DrawRect(x_start, y_start, bar_width * (health / maxHealth), bar_height, {0.8f, 0.1f, 0.1f, 1.0f}, 0.0f);
}

/*
void HUDEntityImpl::DrawClockDisplay() {
    float time = m_clock ? m_clock->GetTimeOfDayHours() : 0.0f;

    int hours = static_cast<int>(time);
    int minutes = static_cast<int>((time - hours) * 60.0f);

    std::string timeStr = std::to_string(hours) + ":" + (minutes < 10 ? "0" : "") + std::to_string(minutes);

    glm::vec4 clockColor = (time > 6.0f && time < 18.0f) ?
                           glm::vec4(1.0f, 1.0f, 0.5f, 1.0f) :
                           glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);

    // TODO: Draw text using your text rendering system
    // DrawText(timeStr, m_screenWidth - 150.0f, m_screenHeight - 50.0f, clockColor);
}
*/

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
