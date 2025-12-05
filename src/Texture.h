    #pragma once

#include "Common.h"

enum TextureBindTarget
{
    TEXTURE_2D = GL_TEXTURE_2D,
    CUBEMAP = GL_TEXTURE_CUBE_MAP
};

class Texture
{
private:
    GLuint m_rendererID;
    const std::string m_targetUniformName;
    GLuint m_slot;

    std::string m_filePath;
    unsigned char *m_localBuffer;
    TextureBindTarget m_target;
    int m_width, m_height, m_BPP;

public:
    Texture(const std::string &path, GLuint slot);
    ~Texture();

    GLuint getID() const { return m_rendererID; }
    GLuint getSlot() const { return m_slot; }

    /**
     * @brief Bind the texture to a new slot.
     */
    void bindNew(GLuint newslot);
    /**
     * @brief Bind the texture to its assigned slot.
     */
    void bind() const;
    void unbind() const;

    std::string targetUniform = "[[[[[[[[[[[[[[NO TARGET UNIFORM SET]]]]]]]]]]]]]]";

    inline int getWidth() const { return m_width; }
    inline int getHeight() const { return m_height; }
};