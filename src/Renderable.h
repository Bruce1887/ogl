#pragma once

#include "Common.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Lighting.h"


// Abstract base class for anything that can be rendered
class Renderable
{
public:
    virtual void render(glm::mat4 view, glm::mat4 projection, PhongLightConfig *phongLight) = 0;
    virtual ~Renderable() = default;

    // Potentially shared among multiple renderables.
    std::shared_ptr<Shader> m_shaderRef;    
    std::vector<Texture *> m_textureReferences;

    inline unsigned int getID() const { return m_ID; }

private:
    unsigned int m_ID = 0;
    GLuint m_VAO_ID = 0;
    // GLuint m_VBO_ID = 0;
    GLuint m_IBO_ID = 0;
    GLuint m_shader_ID = 0;
    std::vector<GLuint> m_texture_IDs;
};

// Anything that exists in the world (3D space) and has a transform
class WorldEntity : public Renderable
{
protected:
    glm::mat4 m_transform = glm::mat4(1.0f); // Initialize to identity
public:
    virtual glm::mat4 getTransform() const
    {
        return m_transform;
    }
    virtual void setTransform(const glm::mat4 &transform)
    {
        m_transform = transform;
    }
    virtual glm::vec3 getPosition() const
    {
        return glm::vec3(m_transform[3]);
    }
    virtual void setPosition(const glm::vec3 &pos)
    {
        m_transform[3] = glm::vec4(pos, 1.0f);
    }
};

// Something that exists flat on the screen and can be rendered, e.g. HUD, health bar, score, menu, etc.
class HUDEntity : public Renderable
{
public:
    glm::vec2 m_position; // 2D position on screen
    glm::vec2 m_size;     // Size on screen
};
