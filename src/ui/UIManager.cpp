#include "UIManager.h"
#include "Skybox.h"
#include <iostream>
#include "game/Audio.h"
#include "game/Database.h"
#include "Common.h"

namespace ui
{

UIManager::UIManager(int screenWidth, int screenHeight)
    : m_currentState(GameState::MAIN_MENU),
      m_isPaused(false),
      m_screenWidth(screenWidth),
      m_screenHeight(screenHeight)
{
    // Create main menu
    m_mainMenu = std::make_unique<MainMenu>(screenWidth, screenHeight);
    
    // Wire up main menu callbacks
    m_mainMenu->onPlayClicked = [this]() {
        DEBUG_PRINT("Play clicked - starting world load..." );
        transitionTo(GameState::LOADING);
    };

    m_mainMenu->onSettingsClicked = [this]() {
        DEBUG_PRINT("Leaderboard clicked" );
        transitionTo(GameState::LEADERBOARD);
    };

    m_mainMenu->onQuitClicked = [this]() {
        DEBUG_PRINT("Quit clicked" );
        if (onQuitGame) {
            onQuitGame();
        }
        transitionTo(GameState::QUITTING);
    };
}

void UIManager::initializeGameUI(Player* player)
{
    m_player = player;

    // Create in-game HUD
    if (player)
    {
        m_gameHUD = std::make_unique<HUDEntityImpl>(player, m_screenWidth, m_screenHeight);
    }
    
    // Create pause menu
    m_pauseMenu = std::make_unique<PauseMenu>(m_screenWidth, m_screenHeight);
    m_pauseMenu->onResumeClicked = [this]() {
        DEBUG_PRINT("Resume clicked" );
        m_isPaused = false;
        SoundPlayer::getInstance().ResumeMusic();
        if (onResumeGame) {
            onResumeGame();
        }
    };
    m_pauseMenu->onSettingsClicked = []() {
        DEBUG_PRINT("Settings clicked from pause menu" );
    };
    m_pauseMenu->onQuitClicked = [this]() {
        DEBUG_PRINT("Quit to menu clicked" );
        m_isPaused = false;
        SoundPlayer::getInstance().StopAll();
        if (onQuitToMenu) {
            onQuitToMenu();
        }
        transitionTo(GameState::MAIN_MENU);
    };
}

void UIManager::setMenuSkybox(std::unique_ptr<Skybox> skybox, std::unique_ptr<Shader> skyboxShader)
{
    m_menuSkybox = std::move(skybox);
    m_menuSkyboxShader = std::move(skyboxShader);
    
    if (m_mainMenu) {
        m_mainMenu->setSkybox(m_menuSkybox.get(), m_menuSkyboxShader.get());
    }
}

void UIManager::update(float deltaTime)
{
    // State-specific update logic
    switch (m_currentState)
    {
        case GameState::LOADING:
            // Call the startup callback once per loading transition
            if (onStartGame && !m_loadingStarted) {
                m_loadingStarted = true;
                onStartGame();
            }
            // Transition to playing once game objects are initialized
            if (m_player && m_gameHUD)
            {
                DEBUG_PRINT("World loaded! Starting gameplay" );
                transitionTo(GameState::PLAYING);
            }
            break;

        case GameState::PLAYING:
            // Check for player death
            checkPlayerDeath();
            break;

        case GameState::DEAD:
            // Update death screen animation
            if (m_deathScreen)
            {
                m_deathScreen->update(deltaTime);
            }
            break;

        default:
            break;
    }
}

void UIManager::checkPlayerDeath()
{
    if (m_player && m_player->m_playerData.m_health <= 0.0f)
    {
        DEBUG_PRINT("Player died!");
        
        // Create death screen if not exists
        if (!m_deathScreen)
        {
            m_deathScreen = std::make_unique<DeathScreen>(m_screenWidth, m_screenHeight);
            m_deathScreen->onSubmitScore = [this](const std::string& playerName, int score) {
                DEBUG_PRINT("Score submitted: " << playerName << " - " << score);
                if (onScoreSubmit) {
                    onScoreSubmit(playerName, score);
                }
                if (onQuitToMenu) {
                    onQuitToMenu();
                }
                transitionTo(GameState::MAIN_MENU);
            };
            m_deathScreen->onContinueClicked = [this]() {
                DEBUG_PRINT("Continue clicked after death (skipped submission)");
                if (onQuitToMenu) {
                    onQuitToMenu();
                }
                transitionTo(GameState::MAIN_MENU);
            };
        }
        
        // Set the player's score on the death screen
        if (m_player) {
            m_deathScreen->setScore(m_player->getScore());
        }
        
        if (onPlayerDied) {
            onPlayerDied();
        }
        
        transitionTo(GameState::DEAD);
    }
}

void UIManager::render(const glm::mat4& view, const glm::mat4& projection)
{
    switch (m_currentState)
    {
        case GameState::MAIN_MENU:{
            if (m_menuSkybox && m_menuSkyboxShader)
            {
                m_menuSkybox->render(view, projection, nullptr);
            }
            if (m_mainMenu)
            {
                m_mainMenu->render(view, projection, nullptr);
            }
            break;
        }
        case GameState::LEADERBOARD:{
            if (!m_leaderboard)
            {
                // Lazy create leaderboard
                m_leaderboard = std::make_unique<Leaderboard>(m_screenWidth, m_screenHeight);
                m_leaderboard->setSkybox(m_menuSkybox.get(), m_menuSkyboxShader.get());
                
                m_leaderboard->onBackClicked = [this]() {
                    DEBUG_PRINT("Back to menu clicked" );
                    transitionTo(GameState::MAIN_MENU);
                };
            }
            
            if (m_leaderboard)
            {
                m_leaderboard->render(view, projection, nullptr);
            }
            break;
        }
        case GameState::LOADING:{
            // Could render loading screen
            //glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            break;
        }
        case GameState::PLAYING:{
            // Render in-game HUD only if not paused
            if (m_gameHUD && !m_isPaused)
            {
                m_gameHUD->render(glm::mat4(1.0f), glm::mat4(1.0f), nullptr);
            }
            
            // Render pause menu overlay if paused (replaces HUD)
            if (m_isPaused && m_pauseMenu)
            {
                m_pauseMenu->render(glm::mat4(1.0f), glm::mat4(1.0f), nullptr);
            }

            break;
        }
        case GameState::DEAD:{
            // Render death screen overlay (world is still visible behind)
            if (m_deathScreen)
            {
                m_deathScreen->render(view, projection, nullptr);
            }
            break;
        }
        default:
            break;
    }
}

void UIManager::handleMouseMove(double mouseX, double mouseY)
{
    switch (m_currentState)
    {
        case GameState::MAIN_MENU:
            if (m_mainMenu) {
                m_mainMenu->handleMouseMove(mouseX, mouseY);
            }
            break;
            
        case GameState::LEADERBOARD:
            if (m_leaderboard) {
                m_leaderboard->handleMouseMove(mouseX, mouseY);
            }
            break;
            
        case GameState::PLAYING:
            if (m_isPaused && m_pauseMenu) {
                m_pauseMenu->handleMouseMove(mouseX, mouseY);
            }
            break;
            
        case GameState::DEAD:
            if (m_deathScreen) {
                m_deathScreen->handleMouseMove(mouseX, mouseY);
            }
            break;
            
        default:
            break;
    }
}

void UIManager::handleMouseClick(double mouseX, double mouseY, bool pressed)
{
    switch (m_currentState)
    {
        case GameState::MAIN_MENU:
            if (m_mainMenu) {
                m_mainMenu->handleMouseClick(mouseX, mouseY, pressed);
            }
            break;
            
        case GameState::LEADERBOARD:
            if (m_leaderboard) {
                m_leaderboard->handleMouseClick(mouseX, mouseY, pressed);
            }
            break;
            
        case GameState::PLAYING:
            if (m_isPaused && m_pauseMenu) {
                m_pauseMenu->handleMouseClick(mouseX, mouseY, pressed);
            }
            break;
            
        case GameState::DEAD:
            if (m_deathScreen) {
                m_deathScreen->handleMouseClick(mouseX, mouseY, pressed);
            }
            break;
            
        default:
            break;
    }
}

void UIManager::handleKeyPress(int key, int action)
{
    // ESC key handling
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if (m_currentState == GameState::PLAYING)
        {
            togglePause();
        }
    }

    // Forward key input to death screen for backspace/enter handling
    if (m_currentState == GameState::DEAD && m_deathScreen)
    {
        m_deathScreen->handleKeyInput(key, action);
    }
}

void UIManager::handleCharInput(unsigned int codepoint)
{
    // Forward character input to death screen for text entry
    if (m_currentState == GameState::DEAD && m_deathScreen)
    {
        m_deathScreen->handleCharInput(codepoint);
    }
}

void UIManager::pollKeyboardInput()
{
    // Handle backspace and enter for death screen text input
    if (m_currentState == GameState::DEAD)
    {
        bool backspacePressed = g_InputManager->keyboardInput.getKeyState(GLFW_KEY_BACKSPACE).readAndClear();
        if (backspacePressed)
        {
            handleKeyPress(GLFW_KEY_BACKSPACE, GLFW_PRESS);
        }
        bool enterPressed = g_InputManager->keyboardInput.getKeyState(GLFW_KEY_ENTER).readAndClear();
        if (enterPressed)
        {
            handleKeyPress(GLFW_KEY_ENTER, GLFW_PRESS);
        }
    }
}

void UIManager::togglePause()
{
    if (m_currentState != GameState::PLAYING)
        return;
        
    m_isPaused = !m_isPaused;
    if (m_isPaused) {
        DEBUG_PRINT("Game paused" );
        SoundPlayer::getInstance().PauseMusic();
    } else {
        DEBUG_PRINT("Game resumed" );
        SoundPlayer::getInstance().ResumeMusic();
        if (onResumeGame) {
            onResumeGame();
        }
    }
}

bool UIManager::shouldShowCursor() const
{
    // Show cursor for menus, hide for gameplay
    return m_currentState == GameState::MAIN_MENU ||
           m_currentState == GameState::LEADERBOARD ||
           m_currentState == GameState::SETTINGS ||
           m_currentState == GameState::DEAD ||
           (m_currentState == GameState::PLAYING && m_isPaused);
}

void UIManager::transitionTo(GameState newState)
{
    if (newState == m_currentState)
        return;

    DEBUG_PRINT("UI: Transitioning from " << (int)m_currentState 
              << " to " << (int)newState );

    m_currentState = newState;
    
    // Reset pause state when leaving PLAYING
    if (newState != GameState::PLAYING) {
        m_isPaused = false;
    }

    if (newState == GameState::PLAYING)
    {
        SoundPlayer::getInstance().ResumeAll();
    }
    else
    {
        SoundPlayer::getInstance().PauseAll();
    }
    
    // Reset game state when returning to main menu
    if (newState == GameState::MAIN_MENU) {
        m_loadingStarted = false;
        m_player = nullptr;
        m_gameHUD.reset();
        m_pauseMenu.reset();
        m_deathScreen.reset();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
    }
    
    // Fetch leaderboard data when entering leaderboard state
    if (newState == GameState::LEADERBOARD) {
        // Ensure leaderboard exists
        if (!m_leaderboard) {
            m_leaderboard = std::make_unique<Leaderboard>(m_screenWidth, m_screenHeight);
            m_leaderboard->setSkybox(m_menuSkybox.get(), m_menuSkyboxShader.get());
            m_leaderboard->onBackClicked = [this]() {
                DEBUG_PRINT("Back to menu clicked");
                transitionTo(GameState::MAIN_MENU);
            };
        }
        
        // Fetch fresh data from database
        m_leaderboard->clearEntries();
        auto entries = Database::FetchTop10();
        for (const auto& entry : entries) {
            int totalSeconds = static_cast<int>(entry.time);
            int minutes = totalSeconds / 60;
            int seconds = totalSeconds % 60;
            char timeStr[16];
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", minutes, seconds);
            m_leaderboard->addEntry(entry.name, entry.kills, timeStr);
        }
        
        if (entries.empty()) {
            m_leaderboard->addEntry("No scores yet!", 0, "--:--");
        }
    }
}

void UIManager::updateScreenSize(int width, int height)
{
    m_screenWidth = width;
    m_screenHeight = height;

    if (m_mainMenu) {
        m_mainMenu->updateScreenSize(width, height);
    }
    if (m_pauseMenu) {
        m_pauseMenu->updateScreenSize(width, height);
    }
    if (m_leaderboard) {
        m_leaderboard->updateScreenSize(width, height);
    }
}

}
