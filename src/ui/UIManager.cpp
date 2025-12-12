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
        std::cout << "Play clicked - starting world load..." << std::endl;
        transitionTo(GameState::LOADING);
    };

    m_mainMenu->onSettingsClicked = [this]() {
        std::cout << "Leaderboard clicked" << std::endl;
        transitionTo(GameState::LEADERBOARD);
    };

    m_mainMenu->onQuitClicked = [this]() {
        std::cout << "Quit clicked" << std::endl;
        if (onQuitGame) {
            onQuitGame();
        }
        transitionTo(GameState::QUITTING);
    };
}

void UIManager::initializeGameUI(Player* player, GameClock* clock)
{
    m_player = player;
    m_gameClock = clock;

    // Create in-game HUD
    if (player && clock)
    {
        m_gameHUD = std::make_unique<HUDEntityImpl>(player, clock, m_screenWidth, m_screenHeight);
    }
    
    // Create pause menu
    m_pauseMenu = std::make_unique<PauseMenu>(m_screenWidth, m_screenHeight);
    m_pauseMenu->onResumeClicked = [this]() {
        std::cout << "Resume clicked" << std::endl;
        m_isPaused = false;
        if (onResumeGame) {
            onResumeGame();
        }
    };
    m_pauseMenu->onSettingsClicked = []() {
        std::cout << "Settings clicked from pause menu" << std::endl;
    };
    m_pauseMenu->onQuitClicked = [this]() {
        std::cout << "Quit to menu clicked" << std::endl;
        m_isPaused = false;
        transitionTo(GameState::MAIN_MENU);
    };
}

void UIManager::setMenuSkybox(Skybox* skybox, Shader* skyboxShader)
{
    m_menuSkybox = skybox;
    m_menuSkyboxShader = skyboxShader;
    
    if (m_mainMenu) {
        m_mainMenu->setSkybox(skybox, skyboxShader);
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
            if (m_player && m_gameClock && m_gameHUD)
            {
                std::cout << "World loaded! Starting gameplay" << std::endl;
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
                m_leaderboard->setSkybox(m_menuSkybox, m_menuSkyboxShader);
                
                // Kommer såklart omfaktureras sen när vi har riktig data
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
                    std::cout << "Back to menu clicked" << std::endl;
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
        std::cout << "Game paused" << std::endl;
    } else {
        std::cout << "Game resumed" << std::endl;
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

    std::cout << "UI: Transitioning from " << (int)m_currentState 
              << " to " << (int)newState << std::endl;

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
