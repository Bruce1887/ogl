#pragma once

#include <glm/glm.hpp>
#include <string>

// Include the full class definitions
#include "../game/Player.h"
#include "../game/GameClock.h"

// Forward declarations for rendering components
class Shader;
class VertexArray;
class VertexBuffer;

class HUD {
public:
    HUD(game::Player* player, game::GameClock* clock, int screenWidth, int screenHeight);
    ~HUD();

    void Draw();
    
private:
    game::Player* m_player;
    game::GameClock* m_clock;

    int m_screenWidth;
    int m_screenHeight;

    Shader* m_hudShader;
    VertexArray* m_quadVAO;
    VertexBuffer* m_quadVBO;

    void DrawRect(float x, float y, float width, float height, const glm::vec4& color);
    void DrawVitalsBars();
    void DrawClockDisplay();
    void SetupQuadRendering();
};
