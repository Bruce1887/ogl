#pragma once

#include "Common.h"


#define MAX_FPS 60


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
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - m_lastFrameTime;
        m_lastFrameTime = currentFrameTime;

        // std::cout << "FPS: " << 1.0f / deltaTime << std::endl;
        return deltaTime;
    }

    float getCurrentTime() const
    {
        return static_cast<float>(glfwGetTime());
    }
};