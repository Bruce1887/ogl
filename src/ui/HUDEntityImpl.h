#pragma once

#include "Renderable.h"
#include "TextRenderer.h"
#include "UIConfig.h"
#include "../game/Player.h"
#include "../game/GameClock.h"

#include <glm/glm.hpp>
#include <string>
#include <memory>


// Renaming the class from HUD to HUDEntityImpl to implement the abstract HUDEntity
class HUDEntityImpl : public HUDEntity {
public:
    HUDEntityImpl(Player* player, int screenWidth, int screenHeight);
    // The destructor is virtual in Renderable, so we use 'override'
    ~HUDEntityImpl() override;

    /**
     * @brief Implementation of the pure virtual render function from Renderable.
     * This is where the 2D drawing logic lives.
     * @param view Ignored for HUD.
     * @param projection Ignored for HUD (we use our own Ortho projection).
     * @param phongLight Ignored for HUD.
     */
    void render(const glm::mat4 view, const glm::mat4 projection, const PhongLightConfig *phongLight) override;
    
    // Set game clock reference for timer display
    void setGameClock(GameClock* clock) { m_gameClock = clock; }
    
    // Set wave number for display
    void setCurrentWave(int wave) { m_currentWave = wave; }
    
    // Update screen size
    void updateScreenSize(int width, int height);
    
private:
    // Game state references
    Player* m_player;
    GameClock* m_gameClock = nullptr;
    int m_currentWave = 0;

    // Screen dimensions used for Orthographic projection setup
    int m_screenWidth;
    int m_screenHeight;

    // Rendering resources
    // m_shaderRef is inherited from Renderable
    std::unique_ptr<VertexArray> m_quadVAO;
    std::unique_ptr<VertexBuffer> m_quadVBO;
    std::unique_ptr<TextRenderer> m_textRenderer;

    // Drawing helpers
    void DrawRect(float x, float y, float width, float height, const glm::vec4& color, float zOffset = 0.0f);
    void DrawText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
    void DrawVitalsBars();
    void DrawTimer();
    void DrawKillsAndWave();
    void SetupQuadRendering();
};