#pragma once

#include "ui/GameState.h"
#include "ui/MainMenu.h"
#include "ui/PauseMenu.h"
#include "ui/Leaderboard.h"
#include "ui/HUDEntityImpl.h"
#include "game/Player.h"
#include "game/GameClock.h"
#include <memory>
#include <functional>
#include <GLFW/glfw3.h>

class Skybox;
class Shader;

namespace ui
{

    /**
     * @brief Manages UI state and transitions between different screens
     *
     * This class coordinates between:
     * - Main menu (before game starts)
     * - Leaderboard view
     * - Loading screen (optional)
     * - In-game HUD (during gameplay)
     * - Pause menu (ESC during gameplay)
     */
    class UIManager
    {
    public:
        UIManager(int screenWidth, int screenHeight);
        ~UIManager() = default;

        // Initialize with game objects (call when starting gameplay)
        void initializeGameUI(Player *player);

        // Set skybox for menu backgrounds
        void setMenuSkybox(std::unique_ptr<Skybox> skybox, std::unique_ptr<Shader> skyboxShader);

        // Update and render based on current state
        void update(float deltaTime);
        void render(const glm::mat4 &view, const glm::mat4 &projection);

        // Input handling
        void handleMouseMove(double mouseX, double mouseY);
        void handleMouseClick(double mouseX, double mouseY, bool pressed);
        void handleKeyPress(int key, int action);

        // State queries and transitions
        GameState getCurrentState() const { return m_currentState; }
        bool isPaused() const { return m_isPaused; }
        void transitionTo(GameState newState);
        void togglePause();

        // Screen size updates
        void updateScreenSize(int width, int height);

        // Cursor mode management
        bool shouldShowCursor() const;

        // Callbacks for menu actions
        std::function<void()> onStartGame;  // Called when "Play" is clicked
        std::function<void()> onQuitGame;   // Called when "Quit" is clicked
        std::function<void()> onResumeGame; // Called when resuming from pause

    private:
        GameState m_currentState;
        bool m_isPaused;
        int m_screenWidth;
        int m_screenHeight;

        // UI components
        std::unique_ptr<MainMenu> m_mainMenu;
        std::unique_ptr<PauseMenu> m_pauseMenu;
        std::unique_ptr<Leaderboard> m_leaderboard;
        std::unique_ptr<HUDEntityImpl> m_gameHUD;

        // References to game objects
        Player *m_player = nullptr;

        // Menu background
        std::unique_ptr<Skybox> m_menuSkybox = nullptr;
        std::unique_ptr<Shader> m_menuSkyboxShader = nullptr;
    };

} // namespace ui
