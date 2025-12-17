// main.cpp - Refactored with WorldManager
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

// Input state management
struct InputState
{
    double cursorX = 0.0;
    double cursorY = 0.0;
    bool escPressedLastFrame = false;
    bool mouseButtonWasPressed = false;
};

// Static input state for GLFW callbacks
static InputState g_inputState;

// GLFW cursor position callback
void cursorPosCallback(GLFWwindow * /*window*/, double xpos, double ypos)
{
    g_inputState.cursorX = xpos;
    g_inputState.cursorY = ypos;
}

int main(int, char **)
{
    // Initialize OpenGL and window
    if (oogaboogaInit(__FILE__))
    {
        return -1;
    }

    {
        // Create UI Manager
        ui::UIManager uiManager(window_X, window_Y);

        // World manager (nullptr until game starts)
        std::unique_ptr<WorldManager> worldManager;

        // Setup menu skybox
        std::vector<std::filesystem::path> menuSkyboxFaces = {
            TEXTURE_DIR / "skybox" / "right.jpg",
            TEXTURE_DIR / "skybox" / "left.jpg",
            TEXTURE_DIR / "skybox" / "top.jpg",
            TEXTURE_DIR / "skybox" / "bottom.jpg",
            TEXTURE_DIR / "skybox" / "front.jpg",
            TEXTURE_DIR / "skybox" / "back.jpg"};
        auto menuSkybox = std::make_unique<Skybox>(menuSkyboxFaces);
        uiManager.setMenuSkybox(std::move(menuSkybox), nullptr);

        // UI callback: Start game
        uiManager.onStartGame = [&]()
        {
            DEBUG_PRINT("Starting game - Initializing world...");

            worldManager = std::make_unique<WorldManager>();

            if (!worldManager->initialize())
            {
                DEBUG_PRINT("Failed to initialize world!");
                worldManager.reset();
                return;
            }

            uiManager.initializeGameUI(
                worldManager->getPlayer(),
                worldManager->getGameClock());

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

        // Set up GLFW cursor callback
        glfwSetCursorPosCallback(g_window, cursorPosCallback);

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
            bool escPressed = glfwGetKey(g_window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
            if (escPressed && !g_inputState.escPressedLastFrame)
            {
                uiManager.handleKeyPress(GLFW_KEY_ESCAPE, GLFW_PRESS);
            }
            g_inputState.escPressedLastFrame = escPressed;

            // Handle mouse input for UI when cursor is visible
            if (shouldShowCursor)
            {
                uiManager.handleMouseMove(g_inputState.cursorX, g_inputState.cursorY);

                bool mousePressed = glfwGetMouseButton(g_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

                // Forward mouse press
                if (mousePressed && !g_inputState.mouseButtonWasPressed)
                {
                    uiManager.handleMouseClick(g_inputState.cursorX, g_inputState.cursorY, true);
                }
                // Forward mouse release
                else if (!mousePressed && g_inputState.mouseButtonWasPressed)
                {
                    uiManager.handleMouseClick(g_inputState.cursorX, g_inputState.cursorY, false);
                }

                g_inputState.mouseButtonWasPressed = mousePressed;
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
            glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render world if playing (even when paused, show world behind menu)
            if (uiManager.getCurrentState() == ui::GameState::PLAYING && worldManager)
            {
                worldManager->render();
            }

            // Render UI on top (disable depth testing, enable blending)
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

        // Cleanup (automatic via unique_ptr destructors)
        worldManager.reset();
    }

    DEBUG_PRINT("Exiting program...");
    oogaboogaExit();
    return 0;
}