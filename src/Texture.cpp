#include "Texture.h"
#include "vendor/stb_image/stb_image.h"


Texture::Texture(const std::string& path, unsigned int slot)
    : m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), 
      m_Width(0), m_Height(0), m_BPP(0), m_Slot(slot)
{
    stbi_set_flip_vertically_on_load(1);
    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
    
    if(!m_LocalBuffer) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return;
    }
    
    GLCALL(glGenTextures(1, &m_RendererID));
    GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_RendererID));
    
    // Wrapping parameters
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
     
    // Texture coordinates do not depend on resolution but can be any floating point value, thus OpenGL has to figure out which texture pixel (also known as a texel) to map the texture coordinate to.
    // Here we decide how the color of a pixel should be decided based on the texture: 
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    // Upload texture data
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 
                        0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));
    

    // Generate mipmaps and set filtering
    GLCALL(glGenerateMipmap(GL_TEXTURE_2D));
    // GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    // GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    // GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
    
    stbi_image_free(m_LocalBuffer);
    m_LocalBuffer = nullptr;
}

Texture::~Texture()
{
    GLCALL(glDeleteTextures(1, &m_RendererID));
}

void Texture::bindNew(unsigned int newslot)
{
    m_Slot = newslot;
    GLCALL(glActiveTexture(GL_TEXTURE0 + newslot));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::bind() const
{
    GLCALL(glActiveTexture(GL_TEXTURE0 + m_Slot));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::unbind() const
{
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

 