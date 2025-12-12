#pragma once

// Serverstyrd

#include <cstdint>
#include <cmath>


// The GameClock is the server-authoritative timekeeper.
// It tracks time-of-day (hours 0..24), the current day phase (DAWN/DAY/DUSK/NIGHT)
// and exposes helpers for ticking time and producing a light intensity value to
// sync to clients (for rendering).
class GameClock
{
public:
    // High-level day phases used by game logic / lighting
    enum class Phase : uint8_t
    {
        DAWN,
        DAY,
        DUSK,
        NIGHT
    };

    // Construct a clock.
    // dayLengthSeconds: real seconds that correspond to one full in-game day (24 hours).
    // startTimeHours: initial time-of-day in hours [0.0, 24.0).
    explicit GameClock(float dayLengthSeconds = 1200.0f, float startTimeHours = 12.0f)
        : CurrentTimeOfDay(normalizeHours(startTimeHours)),
          DayLengthSeconds(dayLengthSeconds),
          CurrentGamePhase(Phase::DAY)
    {
        updatePhase();
    }

    // Advance the clock by deltaTime seconds (call from server game loop).
    // Converts real seconds to in-game hours based on DayLengthSeconds.
    void Update(float deltaTimeSeconds)
    {
        if (DayLengthSeconds <= 0.0f) return; // guard against division by zero
        // convert deltaTime (s) -> hours. 24 hours correspond to DayLengthSeconds seconds.
        float hoursAdvance = (24.0f * deltaTimeSeconds) / DayLengthSeconds;
        CurrentTimeOfDay = normalizeHours(CurrentTimeOfDay + hoursAdvance);
        updatePhase();
    }

    // Returns true when the clock is currently in the NIGHT phase.
    bool IsNight() const { return CurrentGamePhase == Phase::NIGHT; }

    // Returns a light intensity in [0,1] suitable for client-side lighting.
    // Intensity is 0 at full night, 1 at full day, and smoothly blends during dawn/dusk.
    float GetLightIntensity() const
    {
        // Define phase boundaries (hours). Tweak to taste or expose as config.
        constexpr float dawnStart = 6.0f;
        constexpr float dawnEnd   = 8.0f;
        constexpr float dayEnd    = 18.0f;
        constexpr float duskEnd   = 20.0f;
        // Night: [duskEnd .. dawnStart) wraps midnight.

        // Day
        if (CurrentTimeOfDay >= dawnEnd && CurrentTimeOfDay < dayEnd) {
            return 1.0f;
        }

        // Dawn: ramp from 0 -> 1
        if (CurrentTimeOfDay >= dawnStart && CurrentTimeOfDay < dawnEnd) {
            float t = (CurrentTimeOfDay - dawnStart) / (dawnEnd - dawnStart);
            return smoothStep(0.0f, 1.0f, t);
        }

        // Dusk: ramp from 1 -> 0
        if (CurrentTimeOfDay >= dayEnd && CurrentTimeOfDay < duskEnd) {
            float t = (CurrentTimeOfDay - dayEnd) / (duskEnd - dayEnd);
            return smoothStep(1.0f, 0.0f, t);
        }

        // Night (including wrap-around region)
        return 0.0f;
    }

    // Accessors / mutators
    // Current time of day in hours [0,24)
    float GetTimeOfDayHours() const { return CurrentTimeOfDay; }
    // Normalized time [0,1) where 0==0:00 and 1==24:00
    float GetNormalizedTime() const { return CurrentTimeOfDay / 24.0f; }
    Phase GetPhase() const { return CurrentGamePhase; }

    // Force-set time of day (useful for admin commands / testing)
    void SetTimeOfDay(float hours)
    {
        CurrentTimeOfDay = normalizeHours(hours);
        updatePhase();
    }

    // Public data (read-only DayLengthSeconds; CurrentTimeOfDay & CurrentGamePhase are the canonical state)
    float CurrentTimeOfDay;            // in-game hours [0,24)
    const float DayLengthSeconds;     // how many real seconds equals one full 24-hour in-game day
    Phase CurrentGamePhase;           // current phase (DAWN/DAY/DUSK/NIGHT)

private:
    // Normalize hours into [0,24)
    static float normalizeHours(float h)
    {
        float r = std::fmod(h, 24.0f);
        if (r < 0.0f) r += 24.0f;
        return r;
    }

    // Simple smoothstep interpolation between a and b using t in [0,1]
    static float smoothStep(float a, float b, float t)
    {
        if (t <= 0.0f) return a;
        if (t >= 1.0f) return b;
        // smoothstep polynomial
        t = t * t * (3.0f - 2.0f * t);
        return a + (b - a) * t;
    }

    // Update CurrentGamePhase from CurrentTimeOfDay
    void updatePhase()
    {
        // Boundaries chosen to match GetLightIntensity logic
        constexpr float dawnStart = 6.0f;
        constexpr float dawnEnd   = 8.0f;
        constexpr float dayEnd    = 18.0f;
        constexpr float duskEnd   = 20.0f;

        if (CurrentTimeOfDay >= dawnEnd && CurrentTimeOfDay < dayEnd) {
            CurrentGamePhase = Phase::DAY;
            return;
        }
        if (CurrentTimeOfDay >= dayEnd && CurrentTimeOfDay < duskEnd) {
            CurrentGamePhase = Phase::DUSK;
            return;
        }
        if (CurrentTimeOfDay >= dawnStart && CurrentTimeOfDay < dawnEnd) {
            CurrentGamePhase = Phase::DAWN;
            return;
        }
        // otherwise night (covers duskEnd..24 and 0..dawnStart)
        CurrentGamePhase = Phase::NIGHT;
    }
};