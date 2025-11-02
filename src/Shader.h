#pragma once 

#include <glad/glad.h>

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
    UNNASSIGNED = -1
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

    void addShader(const std::string &name, ShaderType type);

    void CreateProgram();
    
    void Bind() const;
    void Unbind() const;

    void SetUniform1i(const std::string &name, int value);
    void SetUniform1f(const std::string &name, float f);
    void SetUniform4f(const std::string &name, std::vector<float> floats);

private:    
    unsigned int CompileShader(ShaderType type, const std::string &source);
    ShaderProgramSource ParseShader(const std::string &filepath, ShaderType type);

    int GetUniformLocation(const std::string &name);
};