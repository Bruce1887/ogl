#pragma once

#include <string>
#include <sstream>
#include <cstdlib>
#include <iostream>

// Simple leaderboard posting using system curl (available on macOS/Linux)
// No external dependencies required!

namespace Leaderboard
{
    inline const std::string FIREBASE_URL = 
        "https://opengl-game-default-rtdb.europe-west1.firebasedatabase.app/leaderboard.json";

    // Post a score to Firebase Realtime Database
    // Returns true if the command was executed (note: doesn't verify server response)
    inline bool PostScore(const std::string& playerName, float timeSurvived, int enemiesKilled)
    {
        // Build JSON payload
        std::stringstream json;
        json << "{"
             << "\"name\":\"" << playerName << "\","
             << "\"time\":" << timeSurvived << ","
             << "\"kills\":" << enemiesKilled
             << "}";

        // Build curl command
        // -s = silent, -X POST = POST request, -H = header, -d = data
        std::stringstream cmd;
        cmd << "curl -s -X POST "
            << "-H \"Content-Type: application/json\" "
            << "-d '" << json.str() << "' "
            << "\"" << FIREBASE_URL << "\" "
            << "> /dev/null 2>&1 &";  // Run in background, discard output

        int result = std::system(cmd.str().c_str());
        
        if (result != 0)
        {
            std::cerr << "Failed to execute curl command" << std::endl;
            return false;
        }

        std::cout << "Score posted: " << playerName 
                  << " - Time: " << timeSurvived 
                  << "s, Kills: " << enemiesKilled << std::endl;
        return true;
    }

    // Post score with wave info
    inline bool PostScore(const std::string& playerName, float timeSurvived, int enemiesKilled, int waveReached)
    {
        std::stringstream json;
        json << "{"
             << "\"name\":\"" << playerName << "\","
             << "\"time\":" << timeSurvived << ","
             << "\"kills\":" << enemiesKilled << ","
             << "\"wave\":" << waveReached
             << "}";

        std::stringstream cmd;
        cmd << "curl -s -X POST "
            << "-H \"Content-Type: application/json\" "
            << "-d '" << json.str() << "' "
            << "\"" << FIREBASE_URL << "\" "
            << "> /dev/null 2>&1 &";

        int result = std::system(cmd.str().c_str());
        return result == 0;
    }
}
