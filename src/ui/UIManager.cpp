#include "UIManager.h"
#include "Skybox.h"
#include <iostream>

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
            // Call the startup callback immediately to initialize the world
            if (onStartGame) {
                onStartGame();
                onStartGame = nullptr;  // Only call once
            }
            // Transition to playing once game objects are initialized
            if (m_player && m_gameHUD)
            {
                DEBUG_PRINT("World loaded! Starting gameplay" );
                transitionTo(GameState::PLAYING);
            }
            break;

        case GameState::PLAYING:
            // Game is running
            break;

        default:
            break;
    }
}

void UIManager::render(const glm::mat4& view, const glm::mat4& projection)
{
    switch (m_currentState)
    {
        case GameState::MAIN_MENU:{
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
                
                // Add sample entries
                m_leaderboard->addEntry("Player One", 5500);
                m_leaderboard->addEntry("Alice", 7200);
                m_leaderboard->addEntry("Bob", 4800);
                m_leaderboard->addEntry("Charlie", 9100);
                m_leaderboard->addEntry("Diana", 6300);
                m_leaderboard->addEntry("Eve", 8200);
                m_leaderboard->addEntry("Frank", 3500);
                m_leaderboard->addEntry("Grace", 7800);
                m_leaderboard->addEntry("Henry", 5200);
                m_leaderboard->addEntry("Ivy", 6900);
                m_leaderboard->addEntry("Jack", 4100);
                
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
}

void UIManager::togglePause()
{
    if (m_currentState != GameState::PLAYING)
        return;
        
    m_isPaused = !m_isPaused;
    if (m_isPaused) {
        DEBUG_PRINT("Game paused" );
    } else {
        DEBUG_PRINT("Game resumed" );
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
