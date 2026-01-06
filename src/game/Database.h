#pragma once

#include <string>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

// Simple database operations using system curl (available on macOS/Linux)
// No external dependencies required!

namespace Database
{
    inline const std::string FIREBASE_URL = 
        "https://opengl-game-default-rtdb.europe-west1.firebasedatabase.app/leaderboard.json";

    // Leaderboard entry structure
    struct Entry
    {
        std::string name;
        float time;
        int kills;
    };

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

    // Fetch top 10 entries by kills (synchronous - blocks until complete)
    // Returns entries sorted by kills (highest first)
    inline std::vector<Entry> FetchTop10()
    {
        std::vector<Entry> entries;
        
        // Temp file to store curl output
        std::string tempFile = "/tmp/leaderboard_response.json";
        
        // Query Firebase: orderBy kills, get last 10 (highest), limitToLast=10
        std::stringstream cmd;
        cmd << "curl -s \""
            << "https://opengl-game-default-rtdb.europe-west1.firebasedatabase.app/leaderboard.json"
            << "?orderBy=\\\"kills\\\"&limitToLast=10"
            << "\" > " << tempFile << " 2>/dev/null";
        
        int result = std::system(cmd.str().c_str());
        if (result != 0)
        {
            std::cerr << "Failed to fetch leaderboard" << std::endl;
            return entries;
        }
        
        // Read response file
        std::ifstream file(tempFile);
        if (!file.is_open())
        {
            std::cerr << "Failed to read leaderboard response" << std::endl;
            return entries;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string response = buffer.str();
        file.close();
        
        // Simple JSON parsing (Firebase returns: { "key1": {...}, "key2": {...} })
        // Look for each entry pattern: "name":"...", "time":..., "kills":...
        size_t pos = 0;
        while ((pos = response.find("\"name\"", pos)) != std::string::npos)
        {
            Entry entry;
            
            // Parse name
            size_t nameStart = response.find("\"", pos + 7) + 1;
            size_t nameEnd = response.find("\"", nameStart);
            if (nameStart != std::string::npos && nameEnd != std::string::npos)
            {
                entry.name = response.substr(nameStart, nameEnd - nameStart);
            }
            
            // Parse time
            size_t timePos = response.find("\"time\"", pos);
            if (timePos != std::string::npos && timePos < pos + 200)
            {
                size_t timeStart = response.find(":", timePos) + 1;
                size_t timeEnd = response.find_first_of(",}", timeStart);
                if (timeStart != std::string::npos && timeEnd != std::string::npos)
                {
                    try {
                        entry.time = std::stof(response.substr(timeStart, timeEnd - timeStart));
                    } catch (...) {
                        entry.time = 0.0f;
                    }
                }
            }
            
            // Parse kills
            size_t killsPos = response.find("\"kills\"", pos);
            if (killsPos != std::string::npos && killsPos < pos + 200)
            {
                size_t killsStart = response.find(":", killsPos) + 1;
                size_t killsEnd = response.find_first_of(",}", killsStart);
                if (killsStart != std::string::npos && killsEnd != std::string::npos)
                {
                    try {
                        entry.kills = std::stoi(response.substr(killsStart, killsEnd - killsStart));
                    } catch (...) {
                        entry.kills = 0;
                    }
                }
            }
            
            entries.push_back(entry);
            pos = nameEnd + 1;
        }
        
        // Sort by kills (highest first) - Firebase limitToLast gives ascending order
        std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
            return a.kills > b.kills;
        });
        
        // Clean up temp file
        std::remove(tempFile.c_str());
        
        return entries;
    }
    
    // Format time as MM:SS
    inline std::string FormatTime(float seconds)
    {
        int mins = static_cast<int>(seconds) / 60;
        int secs = static_cast<int>(seconds) % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
        return std::string(buf);
    }
}
