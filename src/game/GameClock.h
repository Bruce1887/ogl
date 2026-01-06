#pragma once

#include <cstdint>
#include <cmath>
#include <string>

// Simple game clock for tracking elapsed game time.
// Tracks time-of-day in hours (0-24) and total elapsed time.
class GameClock
{
public:
    // Construct a clock.
    // dayLengthSeconds: real seconds that correspond to one full in-game day (24 hours).
    // startTimeHours: initial time-of-day in hours [0.0, 24.0).
    explicit GameClock(float dayLengthSeconds = 1200.0f, float startTimeHours = 12.0f)
        : m_currentTimeOfDay(normalizeHours(startTimeHours)),
          m_dayLengthSeconds(dayLengthSeconds),
          m_totalElapsedSeconds(0.0f)
    {
    }

    // Advance the clock by deltaTime seconds.
    // Converts real seconds to in-game hours based on dayLengthSeconds.
    void Update(float deltaTimeSeconds)
    {
        if (m_dayLengthSeconds <= 0.0f) return;
        
        m_totalElapsedSeconds += deltaTimeSeconds;
        
        // Convert deltaTime (s) -> hours. 24 hours correspond to dayLengthSeconds seconds.
        float hoursAdvance = (24.0f * deltaTimeSeconds) / m_dayLengthSeconds;
        m_currentTimeOfDay = normalizeHours(m_currentTimeOfDay + hoursAdvance);
    }

    // Current time of day in hours [0, 24)
    float GetTimeOfDayHours() const { return m_currentTimeOfDay; }
    
    // Normalized time [0, 1) where 0 == 0:00 and 1 == 24:00
    float GetNormalizedTime() const { return m_currentTimeOfDay / 24.0f; }
    
    // Total real seconds elapsed since clock started
    float GetTotalElapsedSeconds() const { return m_totalElapsedSeconds; }
    
    // Get elapsed time as minutes and seconds (for HUD display)
    int GetElapsedMinutes() const { return static_cast<int>(m_totalElapsedSeconds) / 60; }
    int GetElapsedSeconds() const { return static_cast<int>(m_totalElapsedSeconds) % 60; }
    
    // Get formatted time string "MM:SS" (for HUD/leaderboard)
    std::string GetFormattedTime() const
    {
        int mins = GetElapsedMinutes();
        int secs = GetElapsedSeconds();
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
        return std::string(buf);
    }
    
    // How many real seconds equals one full 24-hour in-game day
    float GetDayLengthSeconds() const { return m_dayLengthSeconds; }

    // Force-set time of day (useful for testing)
    void SetTimeOfDay(float hours)
    {
        m_currentTimeOfDay = normalizeHours(hours);
    }

private:
    float m_currentTimeOfDay;      // in-game hours [0, 24)
    float m_dayLengthSeconds;      // real seconds per in-game day
    float m_totalElapsedSeconds;   // total real time elapsed

    // Normalize hours into [0, 24)
    static float normalizeHours(float h)
    {
        float r = std::fmod(h, 24.0f);
        if (r < 0.0f) r += 24.0f;
        return r;
    }
};