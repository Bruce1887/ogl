#pragma once

#include "../vendor/httplib/httplib.h"

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>


namespace Database
{
    inline const char* FIREBASE_HOST =
        "opengl-game-default-rtdb.europe-west1.firebasedatabase.app";

    inline const char* LEADERBOARD_PATH =
        "/leaderboard.json";

    struct Entry
    {
        std::string name;
        float time;
        int score;
    };

    // POST score to Firebase
    inline bool PostScore(const std::string& playerName, float timeSurvived, int score)
    {
        httplib::SSLClient cli(FIREBASE_HOST);
        cli.set_follow_location(true);

        std::stringstream json;
        json << "{"
             << "\"name\":\"" << playerName << "\","
             << "\"time\":" << timeSurvived << ","
             << "\"score\":" << score
             << "}";

        auto res = cli.Post(
            LEADERBOARD_PATH,
            json.str(),
            "application/json"
        );

        if (!res || res->status >= 300)
        {
            std::cerr << "Failed to post score\n";
            return false;
        }

        return true;
    }

    // Fetch top 10 by score
    inline std::vector<Entry> FetchTop10()
    {
        std::vector<Entry> entries;

        httplib::SSLClient cli(FIREBASE_HOST);
        cli.set_follow_location(true);

        const char* query =
            "/leaderboard.json?orderBy=%22score%22&limitToLast=10";

        auto res = cli.Get(query);

        if (!res || res->status != 200)
        {
            std::cerr << "Failed to fetch leaderboard\n";
            return entries;
        }

        const std::string& response = res->body;

        // --- your existing lightweight JSON parsing ---
        size_t pos = response.find('{');
        if (pos == std::string::npos) return entries;
        pos++;

        while (pos < response.length())
        {
            size_t objStart = response.find('{', pos);
            if (objStart == std::string::npos) break;

            size_t objEnd = response.find('}', objStart);
            if (objEnd == std::string::npos) break;

            std::string entryJson = response.substr(objStart, objEnd - objStart + 1);

            Entry entry{};
            entry.time = 0.0f;
            entry.score = 0;

            size_t namePos = entryJson.find("\"name\"");
            if (namePos != std::string::npos)
            {
                size_t s = entryJson.find("\"", namePos + 6) + 1;
                size_t e = entryJson.find("\"", s);
                if (s != std::string::npos && e != std::string::npos)
                    entry.name = entryJson.substr(s, e - s);
            }

            size_t timePos = entryJson.find("\"time\"");
            if (timePos != std::string::npos)
            {
                size_t s = entryJson.find(":", timePos) + 1;
                size_t e = entryJson.find_first_of(",}", s);
                try { entry.time = std::stof(entryJson.substr(s, e - s)); }
                catch (...) {}
            }

            size_t scorePos = entryJson.find("\"score\"");
            if (scorePos != std::string::npos)
            {
                size_t s = entryJson.find(":", scorePos) + 1;
                size_t e = entryJson.find_first_of(",}", s);
                try { entry.score = std::stoi(entryJson.substr(s, e - s)); }
                catch (...) {}
            }

            if (!entry.name.empty())
                entries.push_back(entry);

            pos = objEnd + 1;
        }

        std::sort(entries.begin(), entries.end(),
            [](const Entry& a, const Entry& b) {
                return a.score > b.score;
            });

        return entries;
    }

    inline std::string FormatTime(float seconds)
    {
        int mins = static_cast<int>(seconds) / 60;
        int secs = static_cast<int>(seconds) % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
        return buf;
    }
}
