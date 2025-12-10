#pragma once

#include "Renderable.h"
#include <glm/glm.hpp>
#include <string>

#include "../game/Player.h"
#include "../game/GameClock.h"

// Renaming the class from HUD to HUDEntityImpl to implement the abstract HUDEntity
class HUDEntityImpl : public HUDEntity {
public:
    HUDEntityImpl(game::Player* player, game::GameClock* clock, int screenWidth, int screenHeight);
    // The destructor is virtual in Renderable, so we use 'override'
    ~HUDEntityImpl() override;

    /**
     * @brief Implementation of the pure virtual render function from Renderable.
     * This is where the 2D drawing logic lives.
     * @param view Ignored for HUD.
     * @param projection Ignored for HUD (we use our own Ortho projection).
     * @param phongLight Ignored for HUD.
     */
    void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) override;
    
private:
    // Game state references
    game::Player* m_player;
    game::GameClock* m_clock;

    // Screen dimensions used for Orthographic projection setup
    int m_screenWidth;
    int m_screenHeight;

    // Rendering resources
    // m_shaderRef is inherited from Renderable
    std::unique_ptr<VertexArray> m_quadVAO;
    std::unique_ptr<VertexBuffer> m_quadVBO;

    // Drawing helpers
    void DrawRect(float x, float y, float width, float height, const glm::vec4& color, float zOffset = 0.0f);
    void DrawVitalsBars();
    void DrawClockDisplay();
    void SetupQuadRendering();
};