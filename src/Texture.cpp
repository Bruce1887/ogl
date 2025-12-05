#include "Texture.h"
#include "vendor/stb_image/stb_image.h"

std::shared_ptr<Texture> Texture::CreateTexture2D(const std::filesystem::path &path, const std::string &targetUniform)
{    
    std::shared_ptr<Texture> tex = std::make_shared<Texture>(TextureBindTarget::TEXTURE_2D);
    tex->m_targetUniform = targetUniform;
    tex->m_filePath = path.string();

    stbi_set_flip_vertically_on_load(1);
    unsigned char* localBuffer = stbi_load(path.c_str(), &tex->m_width, &tex->m_height, &tex->m_BPP, 4);

    assert(localBuffer && "Failed to load texture image!");

    GLCALL(glGenTextures(1, &tex->m_rendererID));
    GLCALL(glActiveTexture(GL_TEXTURE0 + tex->m_slot));
    GLCALL(glBindTexture(GL_TEXTURE_2D, tex->m_rendererID));
    // Wrapping parameters
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    // Texture coordinates do not depend on resolution but can be any floating point value, thus OpenGL has to figure out which texture pixel (also known as a texel) to map the texture coordinate to.
    // Here we decide how the color of a pixel should be decided based on the texture:
    // GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    // GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // Upload texture data
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->m_width, tex->m_height,
                        0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer));
        
    // Generate mipmaps and set filtering
    GLCALL(glGenerateMipmap(GL_TEXTURE_2D));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

    stbi_image_free(localBuffer);
    localBuffer = nullptr;

    return tex;
}

std::shared_ptr<Texture> Texture::CreateCubemap(const std::vector<std::filesystem::path> &facePaths, const std::string &targetUniform)
{
    std::shared_ptr<Texture> tex = std::make_shared<Texture>(TextureBindTarget::CUBEMAP);
    tex->m_targetUniform = targetUniform;

    // Load cubemap texture
    glGenTextures(1, &tex->m_rendererID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex->m_rendererID);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);

    for (unsigned int i = 0; i < facePaths.size(); i++)
    {
        unsigned char *localBuffer = stbi_load(facePaths[i].string().c_str(), &width, &height, &nrChannels, 0);
        
        assert(localBuffer && "Failed to load texture image!");

        if (localBuffer)
        {
            GLenum format = GL_RGB;
            if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, localBuffer);
            stbi_image_free(localBuffer);
        }
        
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return tex;
}

Texture::~Texture()
{
    std::string type = (m_target == TEXTURE_2D) ? "2D Texture" : "Cubemap Texture";
    DEBUG_PRINT("Deleting " << type << " ID " << m_rendererID << " from slot " << m_slot << " with path: " << m_filePath);
    GLCALL(glDeleteTextures(1, &m_rendererID));
}

void Texture::bindNew(GLuint newslot)
{
    m_slot = newslot;
    bind();
}

void Texture::bind() const
{
    std::string type = (m_target == TEXTURE_2D) ? "2D Texture" : "Cubemap Texture";
    std::cout << "Binding " << type << " ID " << m_rendererID << " to slot " << m_slot << " for uniform " << m_targetUniform << std::endl;
    GLCALL(glActiveTexture(GL_TEXTURE0 + m_slot));
    GLCALL(glBindTexture(m_target, m_rendererID));
    RenderingContext *rContext = RenderingContext::Current();
    rContext->m_boundTextures[m_slot] = m_rendererID;
}

void Texture::unbind() const
{
    GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
    RenderingContext *rContext = RenderingContext::Current();
    rContext->m_boundTextures[m_slot] = 0;
}
