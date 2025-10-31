#include "Shader.h"
#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <cassert>

ShaderProgramSource Shader::ParseShader(const std::string &filepath)
{
    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::ifstream stream(filepath);
    if (!stream.is_open())
    {
        std::cerr << "FAILED TO OPEN SHADERFILE AT: " << filepath << std::endl;
        return {"", ""};
    }

    std::string line;
    std::stringstream ss[2];

    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
            else
                std::cerr << __FILE__ << " " << __func__ << " " << __LINE__ << " Invalid shader specified in " << filepath << std::endl;
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return {ss[0].str(), ss[1].str()};
}

unsigned int Shader::CompileShader(unsigned int type, const std::string &source)
{
    unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "FAILED TO COMPILE " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "SHADER!" << std::endl
                  << message << std::endl
                  << std::endl << source << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string &vertexShader, const std::string &fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

Shader::Shader(const std::string &filepath)
    : m_FilePath(filepath), m_RendererID(0)
{
    ShaderProgramSource source = ParseShader(filepath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    GLCALL(glDeleteProgram(m_RendererID));
}

void Shader::Bind() const
{
    GLCALL(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GLCALL(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string &name, int value)
{
    GLCALL(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform4f(const std::string &name, std::vector<float> floats)
{
    assert(floats.size() == 4); // SetUniform4f requires exactly 4 floats

    GLCALL(glUniform4f(GetUniformLocation(name), floats[0], floats[1], floats[2], floats[3]));
}

int Shader::GetUniformLocation(const std::string &name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
    {
        return m_UniformLocationCache[name];
    }
    GLCALL(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if (location == -1)
    {
        std::cerr << "Uniform " << name << " not found!" << std::endl;
    }
    m_UniformLocationCache[name] = location;
    return location;
}
