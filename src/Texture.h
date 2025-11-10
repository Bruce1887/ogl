#pragma once

#include "Renderer.h"

class Texture
{
private:
    unsigned int m_RendererID;
    std::string m_FilePath;
    unsigned char *m_LocalBuffer;
    int m_Width, m_Height, m_BPP;
    unsigned int m_Slot;

public:
    Texture(const std::string &path, unsigned int slot);
    ~Texture();

    unsigned int getID() const { return m_RendererID; }
    void bindNew(unsigned int newslot);
    void bind() const;
    void unbind() const;

    inline int getWidth() const { return m_Width; }
    inline int getHeight() const { return m_Height; }
};