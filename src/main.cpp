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

int main(int, char **)
{
    // Initialize OpenGL and window
    if (oogaboogaInit(__FILE__))
        goto out;
    {
        // Create UI Manager
        ui::UIManager uiManager(WINDOW_X, WINDOW_Y);

        DEBUG_PRINT("Starting in LOADING state just to make things easier for testing...");
        uiManager.transitionTo(ui::GameState::LOADING); // TEMP: Start loading game immediately

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

        // UI callback: Quit game
        uiManager.onQuitGame = [&]()
        {
            glfwSetWindowShouldClose(g_window, GLFW_TRUE);
        };

        // Frame timing
        FrameTimer frameTimer;
        int frameCount = 0;

        // Ensure cursor starts visible (in menu)
        glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        bool lastShouldShowCursor = true;



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
            // Handle mouse input for UI when cursor is visible
            if (shouldShowCursor)
            {
                double cursorX, cursorY;
                g_InputManager->mouseMoveInput.fetchLastPosition(cursorX, cursorY);
                uiManager.handleMouseMove(cursorX, cursorY);

                bool leftMouse, rightMouse;
                g_InputManager->mouseButtonInput.fetchButtons(leftMouse, rightMouse);

                // Forward mouse press
                if (leftMouse)
                {
                    uiManager.handleMouseClick(cursorX, cursorY, false);
                }
                /*
                // Forward mouse release
                else if (!leftMouse && rightMouse)
                {
                    uiManager.handleMouseClick(g_inputState.cursorX, g_inputState.cursorY, false);
                }
                g_inputState.mouseButtonWasPressed = mousePressed;
                */
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
            // Clear screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render world if playing (even when paused, show world behind menu)
            if (uiManager.getCurrentState() == ui::GameState::PLAYING && worldManager)
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

out:
    DEBUG_PRINT("Exiting program...");
    oogaboogaExit();
    return 0;
}