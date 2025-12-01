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

    Renderable()
        : m_ID(s_instanceCounter++)
    {
    }

    // TODO: maybe change visibility
    std::vector<std::shared_ptr<Texture>> m_textureReferences;

    inline unsigned int getID() const { return m_ID; }
    inline unsigned int getRenderableTypeID() const { return m_RenderableTypeID; }

    /**      
     * @brief Set a uniform value specific to this renderable.
     * The uniform isnt actually set until render time where applyUniform is called.
     * 
     * @param name Name of the uniform
     * @param v Value to set the uniform to
     */
    void setUniform(const std::string &name, UniformValue v)
    {
        m_Uniforms[name] = v;
    }

protected:
    // Target uniform values specific to this renderable
    std::unordered_map<std::string, UniformValue> m_Uniforms;

    // Potentially shared among multiple renderables.
    std::shared_ptr<Shader> m_shaderRef;

    /**      
     * @brief Apply a uniform value to the shader associated with this renderable.
     * This actually sets the uniform in the shader, in contrast to setUniform.
     * 
     * @param name Name of the uniform
     * @param value Value to set the uniform to
     * */ 
    void applyUniform(const std::string &name, const UniformValue &value)
    {
        std::visit([this, &name](auto &&val)
                   { m_shaderRef->setUniform(name, val); }, value);
    }
private:
    /**
     * @brief Unique ID for this instance of Renderable (i.e. the ID for a single tree and not all trees)
     * Used for tracking individual instances, e.g. moving enemies or whatever.
     */
    unsigned int m_ID = 0;
    static inline unsigned int s_instanceCounter = 1;

    /**
     * @brief Unique ID for this type of Renderable (i.e. the ID for all pine-tree and not a single pine-trees)
     * Used for optimization purposes (rendering batches of the same type).
     */
    unsigned int m_RenderableTypeID = 0;
    static inline unsigned int s_typeCounter = 1;

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
    glm::mat4 m_transform = glm::mat4(1.0f);

public:
    inline glm::mat4 getTransform() const
    {
        return m_transform;
    }
    inline void setTransform(const glm::mat4 &transform)
    {
        m_transform = transform;
    }
    inline glm::vec3 getPosition() const
    {
        return glm::vec3(m_transform[3]);
    }
    inline void setPosition(const glm::vec3 &pos)
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
