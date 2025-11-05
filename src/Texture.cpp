#include "Texture.h"
#include "vendor/stb_image/stb_image.h"

Texture::Texture(const std::string& path)
    : m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
{
    stbi_set_flip_vertically_on_load(1);
    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);

    if(!m_LocalBuffer) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        goto out;
    }
    
    // Tell OpenGL that you want an identifier for a texture
    GLCALL(glGenTextures(1, &m_RendererID));

    // Bind target "GL_TEXTURE_2D" to our new identifier
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_RendererID));

    // Texture coordinates usually range from (0,0) to (1,1) but what happens if we specify coordinates outside this range? 
    // Thats what we specify here:
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
    // GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    // GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    // Texture coordinates do not depend on resolution but can be any floating point value, thus OpenGL has to figure out which texture pixel (also known as a texel) to map the texture coordinate to.
    // Here we decide how the color of a pixel should be decided based on the texture: 
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // glTexImage2D — specify a two-dimensional texture image
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));

    // Mipmap-grejer. Har att göra med hur pixlar samplar värden från texturer om objektet är långt från kameran.  
    // GLCALL(glGenerateMipmap(GL_TEXTURE_2D));
    // GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    // GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

out:
    if (m_LocalBuffer)
    {
        stbi_image_free(m_LocalBuffer);
    }
}

Texture::~Texture()
{
    GLCALL(glDeleteTextures(1, &m_RendererID));
}


/**
 * @brief 
 * 
 * @param slot defaults to 0 
 */
void Texture::Bind(unsigned int slot) const
{
    GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
    GLCALL(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() const
{
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}

