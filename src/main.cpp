#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <iomanip>

#include "Common.h"
#include "Frametimer.h"
#include "Skybox.h"
#include "WorldManager.h"
#include "ui/UIManager.h"
#include "ui/GameState.h"
#include "Terrain/TerrainChunk.h"
#include "game/Player.h"
#include "game/Enemy.h"
#include "ThirdPersonCamera.h"

int main(int, char **)
{
    // Initialize OpenGL and window
    if (oogaboogaInit(__FILE__))
    {
        printf("OpenGL initialization failed\n");
        return 1;
    }
    {
        // Create UI Manager
        ui::UIManager uiManager(WINDOW_X, WINDOW_Y);
        uiManager.onQuitGame = []() {
            DEBUG_PRINT("Quit button pressed - closing window...");
            glfwSetWindowShouldClose(g_window, GLFW_TRUE);
        };

        DEBUG_PRINT("Starting in MAIN_MENU state...");
        uiManager.transitionTo(ui::GameState::MAIN_MENU); 

        // World manager (nullptr until game starts)
        std::unique_ptr<WorldManager> worldManager;

        // Setup menu skybox
        auto menuSkybox = std::make_unique<Skybox>();
        uiManager.setMenuSkybox(std::move(menuSkybox), nullptr);

        // UI callback: Start game
        uiManager.onStartGame = [&]()
        {
            DEBUG_PRINT("Initializing world...");
            worldManager = std::make_unique<WorldManager>();
            if (!worldManager->initialize())
            {
                DEBUG_PRINT("Failed to initialize world!");
                worldManager.reset();
                return;
            }
            uiManager.initializeGameUI(
                worldManager->getPlayer());
            DEBUG_PRINT("World initialized successfully!");
        };

        // UI callback: Resume game
        uiManager.onResumeGame = [&]()
        {
            // Cursor mode is handled automatically by UIManager
        };

        // UI callback: Quit to main menu (pause menu)
        uiManager.onQuitToMenu = [&]()
        {
            worldManager.reset();
        };

        // Cleanup of worldManager when leaving PLAYING handled in main loop

        // Frame timing
        FrameTimer frameTimer;
        int frameCount = 0;

        // Ensure cursor starts visible (in menu)
        glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        bool lastShouldShowCursor = true;

        // Set up character callback for text input (death screen name entry)
        glfwSetWindowUserPointer(g_window, &uiManager);
        glfwSetCharCallback(g_window, [](GLFWwindow* window, unsigned int codepoint) {
            auto* ui = static_cast<ui::UIManager*>(glfwGetWindowUserPointer(window));
            if (ui) {
                ui->handleCharInput(codepoint);
            }
        });

        // Main game loop
        while (!glfwWindowShouldClose(g_window))
        {
            float dt = frameTimer.getDeltaTime();

            glfwPollEvents();

            // Update cursor mode based on UI state
            bool shouldShowCursor = uiManager.shouldShowCursor();
            if (shouldShowCursor != lastShouldShowCursor)
            {
                glfwSetInputMode(
                    g_window,
                    GLFW_CURSOR,
                    shouldShowCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
                lastShouldShowCursor = shouldShowCursor;
            }

            // Handle ESC key for pause/menu
            bool escPressed = g_InputManager->keyboardInput.getKeyState(GLFW_KEY_ESCAPE).readAndClear();
            if (escPressed)
            {
                uiManager.handleKeyPress(GLFW_KEY_ESCAPE, GLFW_PRESS);
            }

            // Poll for special keys (backspace, enter) for text input
            uiManager.pollKeyboardInput();

            // Handle mouse input for UI when cursor is visible
            if (shouldShowCursor)
            {
                double cursorX, cursorY;
                g_InputManager->mouseMoveInput.fetchLastPosition(cursorX, cursorY);
                uiManager.handleMouseMove(cursorX, cursorY);

                static bool lastLeftMouse = false;
                bool leftMouse, rightMouse;
                g_InputManager->mouseButtonInput.fetchButtons(leftMouse, rightMouse);

                if (leftMouse && !lastLeftMouse)
                {
                    uiManager.handleMouseClick(cursorX, cursorY, true);
                }

                // mouse released
                if (!leftMouse && lastLeftMouse)
                {
                    uiManager.handleMouseClick(cursorX, cursorY, false);
                }

                lastLeftMouse = leftMouse;
            }

            // Update UI
            uiManager.update(dt); 

            // Update game world if playing and not paused
            if (uiManager.getCurrentState() == ui::GameState::PLAYING &&
                !uiManager.isPaused() &&
                worldManager)
            {
                worldManager->update(dt, g_InputManager);
            }

            // === RENDERING ===
            // Clear screen with a visible color (not just black)
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render world if playing or dead (show world behind death screen)
            ui::GameState currentState = uiManager.getCurrentState();
            if ((currentState == ui::GameState::PLAYING || currentState == ui::GameState::DEAD) && worldManager)
            {
                worldManager->render();
            }

            uiManager.render(glm::mat4(1.0f), glm::mat4(1.0f));

            // Print stats periodically
            if (worldManager &&
                frameCount % 120 == 0 &&
                uiManager.getCurrentState() == ui::GameState::PLAYING &&
                !uiManager.isPaused())
            {

                auto *chunkManager = worldManager->getChunkManager();
                if (chunkManager)
                {
                    DEBUG_PRINT("Chunks: " << chunkManager->m_chunks.size()
                                           << " | FPS: " << std::fixed << std::setprecision(1)
                                           << (1.0f / dt));
                }
            }
            frameCount++;
            glfwSwapBuffers(g_window);
        }
    }

    DEBUG_PRINT("Exiting program...");
    oogaboogaExit();
    return 0;
}
