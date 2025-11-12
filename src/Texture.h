#pragma once

#include "Common.h"

class Texture
{
private:
    GLuint m_rendererID;
    const std::string m_targetUniformName;
    GLuint m_slot;

    std::string m_filePath;
    unsigned char *m_localBuffer;
    int m_width, m_height, m_BPP;

public:
    Texture(const std::string &path, GLuint slot);
    ~Texture();

    GLuint getID() const { return m_rendererID; }
    GLuint getSlot() const { return m_slot; }
    void bindNew(GLuint newslot);
    void bind() const;
    void unbind() const;

    std::string targetUniform = "BAD_UNIFORM_NAME";

    inline int getWidth() const { return m_width; }
    inline int getHeight() const { return m_height; }
};