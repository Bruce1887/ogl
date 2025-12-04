
#include "../game/Player.h"    
#include "../game/GameClock.h"  

#include "HUD.h"
#include "TextRenderer.h"

// OpenGL libraries
#include "../glad/glad.h" 
#include "../Common.h"
#include "Shader.h"

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

// Include OpenGL rendering core components (Shader, VertexArray, etc.)

// Constructor Implementation
HUD::HUD(game::Player* player, game::GameClock* clock, int screenWidth, int screenHeight)
    : m_player(player), m_clock(clock), m_screenWidth(screenWidth), m_screenHeight(screenHeight), 
      m_hudShader(nullptr), m_quadVAO(nullptr), m_quadVBO(nullptr), m_textRenderer(nullptr)
{
    // 1. Initialize Rendering Resources (Shaders and buffers)
    // Load your basic 2D color shader (uniformColor.frag/vert from resources/shaders)
    m_hudShader = new Shader();
    m_hudShader->addShader("2D.vert", ShaderType::VERTEX); 
    m_hudShader->addShader("uniformColor.frag", ShaderType::FRAGMENT);
    m_hudShader->createProgram();
    m_hudShader->bind();

    // 2. Setup the buffer required to draw a single 2D quad
    SetupQuadRendering(); 
}

HUD::~HUD() {
    // Clean up allocated resources
    delete m_hudShader;
    delete m_quadVAO;
    delete m_quadVBO;
    delete m_textRenderer;
}

bool HUD::initTextRenderer(const std::string& fontPath, unsigned int fontSize)
{
    if (m_textRenderer) {
        delete m_textRenderer;
    }
    m_textRenderer = new TextRenderer();
    return m_textRenderer->init(fontPath, fontSize, m_screenWidth, m_screenHeight);
}


void HUD::Draw() {
    // CRUCIAL STEP 1: Disable Depth Testing
    // The HUD is always on top, so depth testing (z-buffer) must be off.
    GLCALL(glDisable(GL_DEPTH_TEST));
    
    // CRUCIAL STEP 2: Set Orthographic Projection
    // This transforms the coordinate system to 2D screen space (0,0 bottom-left, W,H top-right)
    glm::mat4 projection = glm::ortho(0.0f, (float)m_screenWidth, 0.0f, (float)m_screenHeight);
    
    // Bind the shader and set the projection matrix
    m_hudShader->bind();
    m_hudShader->setUniform("u_Projection", projection);

    // Render components
    DrawVitalsBars();
    DrawClockDisplay();

    // CRUCIAL STEP 3: Re-enable Depth Testing for 3D rendering next frame
    GLCALL(glEnable(GL_DEPTH_TEST));
}

// --- COMPONENT LOGIC ---

void HUD::DrawVitalsBars() {
    // Access Health (using the getter 'getHealth()' from Entity.h)
    float health = m_player ? m_player->getHealth() : 0.0f;
    float maxHealth = 100.0f;
    float stamina = m_player ? m_player->getStamina() : 0.0f;
    float maxStamina = 100.0f; // Since defaultStamina in Player.h is 100.0f
    
    float x_start = 50.0f;
    float y_start = 50.0f;
    float bar_width = 200.0f;
    float bar_height = 20.0f;
    
    // 1. Draw Health Bar
    DrawRect(x_start, y_start, bar_width, bar_height, {0.3f, 0.0f, 0.0f, 1.0f});
    float healthFill = bar_width * (health / maxHealth);
    DrawRect(x_start, y_start, healthFill, bar_height, {0.8f, 0.1f, 0.1f, 1.0f});
    
    // Draw health label and value if text renderer is available
    if (m_textRenderer && m_textRenderer->isInitialized()) {
        std::string healthStr = std::to_string(static_cast<int>(health)) + "/" + std::to_string(static_cast<int>(maxHealth));
        DrawText(healthStr, x_start + bar_width + 10.0f, y_start + 2.0f, {1.0f, 1.0f, 1.0f, 1.0f}, 0.5f);
    }

    // 2. Draw Stamina Bar
    y_start += bar_height + 10.0f;
    DrawRect(x_start, y_start, bar_width, bar_height, {0.0f, 0.3f, 0.0f, 1.0f});
    float staminaFill = bar_width * (stamina / maxStamina);
    DrawRect(x_start, y_start, staminaFill, bar_height, {0.1f, 0.8f, 0.1f, 1.0f});
    
    // Draw stamina label and value if text renderer is available
    if (m_textRenderer && m_textRenderer->isInitialized()) {
        std::string staminaStr = std::to_string(static_cast<int>(stamina)) + "/" + std::to_string(static_cast<int>(maxStamina));
        DrawText(staminaStr, x_start + bar_width + 10.0f, y_start + 2.0f, {1.0f, 1.0f, 1.0f, 1.0f}, 0.5f);
    }
}

void HUD::DrawClockDisplay() {
    float time = m_clock ? m_clock->GetTimeOfDayHours() : 0.0f;    

    // Calculate display time (e.g., convert float hours to HH:MM format)
    int hours = static_cast<int>(time);
    int minutes = static_cast<int>((time - hours) * 60.0f);
    
    std::string timeStr = std::to_string(hours) + ":" + (minutes < 10 ? "0" : "") + std::to_string(minutes);

    // Determine color based on day/night cycle
    glm::vec4 clockColor = (time > 6.0f && time < 18.0f) ? 
                           glm::vec4(1.0f, 1.0f, 0.5f, 1.0f) : // Day (Yellowish)
                           glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);  // Night (Bluish)

    // Draw the timeStr text in the top right corner
    DrawText(timeStr, static_cast<float>(m_screenWidth) - 150.0f, static_cast<float>(m_screenHeight) - 50.0f, clockColor);
}

void HUD::DrawText(const std::string& text, float x, float y, const glm::vec4& color, float scale)
{
    if (m_textRenderer && m_textRenderer->isInitialized()) {
        m_textRenderer->RenderText(text, x, y, scale, color);
        
        // Re-bind HUD shader after text rendering
        m_hudShader->bind();
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(m_screenWidth), 0.0f, static_cast<float>(m_screenHeight));
        m_hudShader->setUniform("u_Projection", projection);
    }
}

// --- BASIC RECTANGLE DRAWING IMPLEMENTATION ---

void HUD::DrawRect(float x, float y, float width, float height, const glm::vec4& color) {
    // This function needs to:
    // 1. Calculate a Model matrix (Translation + Scale) to position the quad
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));

    // 2. Bind the shader, set uniforms (Model matrix and Color)
    m_hudShader->bind();
    m_hudShader->setUniform("u_Model", model);
    m_hudShader->setUniform("u_color", color);    
    // 3. Draw the quad
    m_quadVAO->bind();
    GLCALL(glDrawArrays(GL_TRIANGLES, 0, 6)); // Assuming a 6-vertex quad
}

// --- QUAD BUFFER SETUP ---
void HUD::SetupQuadRendering() {
    // This defines a single quad that will be scaled/translated by the DrawRect function
    float vertices[] = {
        // positions (normalized to 1x1)
        0.0f, 1.0f, // Top-left
        0.0f, 0.0f, // Bottom-left
        1.0f, 0.0f, // Bottom-right
        
        0.0f, 1.0f, // Top-left
        1.0f, 0.0f, // Bottom-right
        1.0f, 1.0f  // Top-right
    };
    // 1. Initialize VAO FIRST, so we have a pointer to pass
    m_quadVAO = new VertexArray(); 
    
    // 2. Initialize VBO using the 4-argument constructor.
    // Args: data, size, associatedVAO, usage
    // We pass m_quadVAO and let usage default to STATIC_DRAW.
    
    // NOTE: You must cast the size to unsigned int (as required by your VBO header)
    unsigned int bufferSize = (unsigned int)sizeof(vertices);
    
    m_quadVBO = new VertexBuffer(
        vertices,               // const void* data
        bufferSize,             // unsigned int size
        m_quadVAO               // VertexArray* associatedVAO (The key missing argument)
        // BufferUsage will default to STATIC_DRAW
    );
    
    // 3. Define Layout
    VertexBufferLayout layout;
    layout.push<float>(2); // 2D Position
    
    // 4. Link VBO to VAO (This step is often redundant if the VBO constructor handles it, 
    // but you need it here to ensure the layout is set.)
    m_quadVAO->addBuffer(m_quadVBO, layout);
}
