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
        int score;
    };

    // Post a score to Firebase Realtime Database
    // Returns true if the command was executed (note: doesn't verify server response)
    inline bool PostScore(const std::string& playerName, float timeSurvived, int score)
    {
        // Build JSON payload
        std::stringstream json;
        json << "{"
             << "\"name\":\"" << playerName << "\","
             << "\"time\":" << timeSurvived << ","
             << "\"score\":" << score
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
                  << "s, score: " << score << std::endl;
        return true;
    }

    // Fetch top 10 entries by score (synchronous - blocks until complete)
    // Returns entries sorted by score (highest first)
    inline std::vector<Entry> FetchTop10()
    {
        std::vector<Entry> entries;
        
        // Temp file to store curl output
        std::string tempFile = "/tmp/leaderboard_response.json";
        
        // Query Firebase: orderBy "score", get last 10 (highest), limitToLast=10
        std::stringstream cmd;
        cmd << "curl -s \""
            << "https://opengl-game-default-rtdb.europe-west1.firebasedatabase.app/leaderboard.json"
            << "?orderBy=\\\"score\\\"&limitToLast=10"
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
        // Find each nested object by looking for opening/closing braces within the main object
        // Skip the outermost braces
        size_t pos = response.find('{');
        if (pos == std::string::npos) return entries;
        pos++; // Skip outer opening brace
        
        while (pos < response.length())
        {
            // Find next nested object (starts after a colon following the Firebase key)
            size_t objStart = response.find('{', pos);
            if (objStart == std::string::npos) break;
            
            // Find matching closing brace for this object
            size_t objEnd = response.find('}', objStart);
            if (objEnd == std::string::npos) break;
            
            // Extract just this entry's JSON object
            std::string entryJson = response.substr(objStart, objEnd - objStart + 1);
            
            Entry entry;
            entry.time = 0.0f;
            entry.score = 0;
            
            // Parse name within this entry
            size_t namePos = entryJson.find("\"name\"");
            if (namePos != std::string::npos)
            {
                size_t nameStart = entryJson.find("\"", namePos + 6) + 1;
                size_t nameEnd = entryJson.find("\"", nameStart);
                if (nameStart != std::string::npos && nameEnd != std::string::npos && nameStart < nameEnd)
                {
                    entry.name = entryJson.substr(nameStart, nameEnd - nameStart);
                }
            }
            
            // Parse time within this entry
            size_t timePos = entryJson.find("\"time\"");
            if (timePos != std::string::npos)
            {
                size_t timeStart = entryJson.find(":", timePos) + 1;
                size_t timeEnd = entryJson.find_first_of(",}", timeStart);
                if (timeStart != std::string::npos && timeEnd != std::string::npos)
                {
                    try {
                        entry.time = std::stof(entryJson.substr(timeStart, timeEnd - timeStart));
                    } catch (...) {
                        entry.time = 0.0f;
                    }
                }
            }
            
            // Parse score within this entry
            size_t scorePos = entryJson.find("\"score\"");
            if (scorePos != std::string::npos)
            {
                size_t scoreStart = entryJson.find(":", scorePos) + 1;
                size_t scoreEnd = entryJson.find_first_of(",}", scoreStart);
                if (scoreStart != std::string::npos && scoreEnd != std::string::npos)
                {
                    try {
                        entry.score = std::stoi(entryJson.substr(scoreStart, scoreEnd - scoreStart));
                    } catch (...) {
                        entry.score = 0;
                    }
                }
            }
            
            // Only add if we got a valid name
            if (!entry.name.empty())
            {
                entries.push_back(entry);
            }
            
            pos = objEnd + 1;
        }
        
        // Sort by score(highest first) - Firebase limitToLast gives ascending order
        std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
            return a.score > b.score;
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
