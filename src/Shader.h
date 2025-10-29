#pragma once 

#include <string>
#include <vector>
#include <unordered_map>

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

class Shader
{
private:
    std::string m_FilePath;
    unsigned int m_RendererID; // Unique ID for the buffer
    std::unordered_map<std::string, int> m_UniformLocationCache;

public:
    Shader(const std::string &filepath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    void SetUniform1i(const std::string &name, int value);

    void SetUniform4f(const std::string &name, std::vector<float> floats);

private:    
    unsigned int CreateShader(const std::string &vertexShader, const std::string &fragmentShader);
    unsigned int CompileShader(unsigned int type, const std::string &source);
    ShaderProgramSource ParseShader(const std::string &filepath);

    int GetUniformLocation(const std::string &name);
};