#pragma once

#include "ui/GameState.h"
#include "ui/MainMenu.h"
#include "ui/PauseMenu.h"
#include "ui/Leaderboard.h"
#include "ui/LoadingScreen.h"
#include "ui/HUDEntityImpl.h"
#include "ui/DeathScreen.h"
#include "game/Player.h"
#include "game/GameClock.h"
#include <AL/al.h>
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
        void handleCharInput(unsigned int codepoint);
        void pollKeyboardInput();  // Call each frame to handle special keys (backspace, enter) for text input

        // State queries and transitions
        GameState getCurrentState() const { return m_currentState; }
        bool isPaused() const { return m_isPaused; }
        void transitionTo(GameState newState);
        void togglePause();
        
        // Check if player is dead and trigger death screen
        void checkPlayerDeath();

        // Screen size updates
        void updateScreenSize(int width, int height);

        // Cursor mode management
        bool shouldShowCursor() const;

        // Callbacks for menu actions
        std::function<void()> onStartGame;  // Called when "Play" is clicked
        std::function<void()> onQuitGame;   // Called when "Quit" is clicked
        std::function<void()> onQuitToMenu; // Called when quitting to main menu from pause
        std::function<void()> onResumeGame; // Called when resuming from pause
        std::function<void()> onPlayerDied; // Called when player dies
        std::function<void()> onGameplayStarted; // Called when transitioning to PLAYING (after loading)
        std::function<void(const std::string& playerName, int score)> onScoreSubmit; // Called when score is submitted

        // Update HUD with game state (call each frame during gameplay)
        void updateHUDState(GameClock* gameClock, int currentWave);

    private:
        GameState m_currentState;
        bool m_isPaused;
        bool m_loadingStarted = false;  // Track if loading callback was called
        int m_loadingFrames = 0;        // Count frames in loading state (to show screen before starting load)
        bool m_deathSoundPlayed = false;  // Track if death sound was played
        int m_screenWidth;
        int m_screenHeight;

        // UI components
        std::unique_ptr<MainMenu> m_mainMenu;
        std::unique_ptr<PauseMenu> m_pauseMenu;
        std::unique_ptr<Leaderboard> m_leaderboard;
        std::unique_ptr<LoadingScreen> m_loadingScreen;
        std::unique_ptr<HUDEntityImpl> m_gameHUD;
        std::unique_ptr<DeathScreen> m_deathScreen;

        // References to game objects
        Player *m_player = nullptr;

        // Menu background
        std::unique_ptr<Skybox> m_menuSkybox = nullptr;
        std::unique_ptr<Shader> m_menuSkyboxShader = nullptr;
    };

} // namespace ui
