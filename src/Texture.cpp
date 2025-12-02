#include "Texture.h"
#include "vendor/stb_image/stb_image.h"

Texture::Texture(const std::string &path, unsigned int slot)
    : m_rendererID(0), m_filePath(path), m_localBuffer(nullptr),
      m_width(0), m_height(0), m_BPP(0), m_slot(slot)
{
    stbi_set_flip_vertically_on_load(1);
    m_localBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_BPP, 4);

    if (!m_localBuffer)
    {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return;
    }

    GLCALL(glGenTextures(1, &m_rendererID));
    GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererID));

    // Wrapping parameters
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    // Texture coordinates do not depend on resolution but can be any floating point value, thus OpenGL has to figure out which texture pixel (also known as a texel) to map the texture coordinate to.
    // Here we decide how the color of a pixel should be decided based on the texture:
    // GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    // GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // Upload texture data
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height,
                        0, GL_RGBA, GL_UNSIGNED_BYTE, m_localBuffer));

    // Generate mipmaps and set filtering
    GLCALL(glGenerateMipmap(GL_TEXTURE_2D));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

    stbi_image_free(m_localBuffer);
    m_localBuffer = nullptr;
}

Texture::~Texture()
{
#ifdef DEBUG
    DEBUG_PRINT("Deleting texture ID " << m_rendererID << " from slot " << m_slot << " with path: " << m_filePath);
#endif
    GLCALL(glDeleteTextures(1, &m_rendererID));
}

void Texture::bindNew(GLuint newslot)
{
    m_slot = newslot;
    bind();
}

void Texture::bind() const
{
    GLCALL(glActiveTexture(GL_TEXTURE0 + m_slot));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_rendererID));
    RenderingContext *rContext = RenderingContext::Current();
    rContext->m_boundTextures[m_slot] = m_rendererID;
}

void Texture::unbind() const
{
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
    RenderingContext *rContext = RenderingContext::Current();
    rContext->m_boundTextures[m_slot] = 0;
}
