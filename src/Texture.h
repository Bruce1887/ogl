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
    GLuint m_slot;

    std::string m_filePath;    
    TextureBindTarget m_target;
    int m_width, m_height, m_BPP;

public:
    /**      
     * @brief Constructor to enforce use of static creation methods. 
     * This is only public to allow use of std::make_shared in the static methods. // TODO: FIND SOME BETTER WAY TO GIVE ACCESS TO CONSTRUCTOR IN STATIC METHODS
     * 
     * @param target Texture binding target (e.g. GL_TEXTURE_2D)
     */
    Texture(TextureBindTarget target)
        : m_target(target)
    {
        m_slot = 0;
        m_targetUniform = "[NO UNIFORM SPECIFIED]";
        m_filePath = "[NO FILEPATH SPECIFIED]";
    }    
    ~Texture();

    static std::shared_ptr<Texture> CreateTexture2D(const std::filesystem::path &path, const std::string &targetUniform);
    static std::shared_ptr<Texture> CreateCubemap(const std::vector<std::filesystem::path> &facePaths, const std::string &targetUniform);

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

    std::string m_targetUniform;

    inline int getWidth() const { return m_width; }
    inline int getHeight() const { return m_height; }
};