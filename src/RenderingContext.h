#pragma once

#include <glad/glad.h>
#include <iostream>

/**
 * RenderingContext struct to keep track of currently bound OpenGL resources (among other things).
 * This helps minimize redundant OpenGL state changes by tracking what is already bound.
 * 
 * This class is designed as a singleton, with one global instance accessible via RenderingContext::Current().
 * After creating a RenderingContext instance, call makeCurrent() to set it as the active context.
 */
class RenderingContext
{
private:
    static RenderingContext* s_current;  // ONE shared pointer for ALL instances

public:
    // Get the current one
    static RenderingContext* Current() { 
        #ifdef DEBUG
        if (s_current == nullptr) {
            std::cerr << "Warning: No current RenderingContext set!" << std::endl;
        }
        #endif
        return s_current;
    }

    void makeCurrent() { s_current = this; }  // Set THIS instance as current

    GLuint m_boundTextures[32] = {0}; // Assuming exactly 32 texture slots
    GLuint m_boundShader = 0;
    GLuint m_boundVAO = 0;
    GLuint m_boundIBO = 0;
    // GLuint m_boundVBO = 0;
};