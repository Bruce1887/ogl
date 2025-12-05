#pragma once

#include "Common.h"

class FrameTimer
{
private:
    float m_lastFrameTime = 0.0f;
public:

    /** 
     * @brief Returns the time elapsed since the last call to getDeltaTime in seconds. Updates timestamp of last frame.
     * @return float Delta time in seconds.
     */
    float getDeltaTime()
    {
        float currentFrameTime = getCurrentTime();
        float deltaTime = currentFrameTime - m_lastFrameTime;
        m_lastFrameTime = currentFrameTime;

        return deltaTime;
    }

    float getCurrentTime() const
    {
        return static_cast<float>(glfwGetTime());
    }
};