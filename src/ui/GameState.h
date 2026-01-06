#pragma once

namespace ui
{

// Represents the current state of the application/game
enum class GameState
{
    MAIN_MENU,      // Main menu screen (play, settings, quit)
    LEADERBOARD,    // Leaderboard view (from main menu)
    SETTINGS,       // Settings menu
    LOADING,        // Loading screen while world generates
    PLAYING,        // Active gameplay
    PAUSED,         // Game paused (ESC menu)
    DEAD,           // Player died - show death screen
    QUITTING        // Shutting down
};

} // namespace ui
