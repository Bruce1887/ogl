#pragma once 

#include "Common.h"

#include <string>
#include <vector>
#include <unordered_map>

enum class ShaderType
{
    VERTEX =  GL_VERTEX_SHADER,
    FRAGMENT = GL_FRAGMENT_SHADER,
    COMPUTE = GL_COMPUTE_SHADER,
    TESS_CONTROL =  GL_TESS_CONTROL_SHADER,
    TESS_EVAL = GL_TESS_EVALUATION_SHADER,
    GEOMETRY =  GL_GEOMETRY_SHADER,
    UNASSIGNED = -1
};

struct ShaderProgramSource
{
    std::string content; // non compiled shader in string form
    ShaderType type; // e.g. vertex shader or something else
};

class Shader // två "private" i class def, hur snyggt är det
{
private:
    std::vector<ShaderProgramSource> m_programSources;
    unsigned int m_RendererID; // Unique ID for the buffer
    std::unordered_map<std::string, int> m_UniformLocationCache;

public:
    Shader();
    ~Shader();

    unsigned int getID() const { return m_RendererID; }

    void addShader(const std::string &name, ShaderType type);

    void createProgram();
    
    void bind() const;
    void unbind() const;

    void setUniform(const std::string &name, int value);
    void setUniform(const std::string &name, float value);
    void setUniform(const std::string &name, const glm::vec2 &v);
    void setUniform(const std::string &name, const glm::vec3 &v);
    void setUniform(const std::string &name, const glm::vec4 &v);
    void setUniform(const std::string &name, const glm::mat4 &m);


private:    
    
    /**
     * @brief 
     * 
     * @param type Det är här typen av shadern faktist kommer till användning i "glCreateShader((int)type)"
     * @param source 
     * @return unsigned int 
     */
    unsigned int compileShader(ShaderType type, const std::string &source);
    
    /**
     * @brief Läs shaderfilen och samlar datan som behövs för att kompilera och binda ett shaderprogram rätt.
     * 
     * @param filepath filsökväg till en shader (e.g. .vert eller .frag)
     * @param type shadertyp, e.g. vertex shader eller fragment shader
     * @return ShaderProgramSource Det som behövs för att kompilera och binda ett shaderprogram rätt 
     */
    ShaderProgramSource parseShader(const std::string &filepath, ShaderType type);

    int getUniformLocation(const std::string &name);
};