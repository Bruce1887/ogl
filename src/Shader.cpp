#include "Shader.h"

#include <Renderer.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <cassert>

ShaderProgramSource Shader::ParseShader(const std::string &filepath, ShaderType type)
{
    std::ifstream stream(filepath);
    if (!stream.is_open())
    {
        std::cerr << "FAILED TO OPEN SHADERFILE AT: " << filepath << std::endl;
        return {"", ShaderType::UNNASSIGNED};
    }

    std::string line;
    std::stringstream ss;

    // Read content line by line, stupid
    while (getline(stream, line))
    {
        ss << line << '\n';
    }

    return {ss.str(), type};
}

unsigned int Shader::CompileShader(ShaderType type, const std::string &source)
{
    unsigned int id = glCreateShader((int)type);
#ifdef DEBUG
    if (id == 0)
    {
        std::cerr << "Could not create shader with type: " << type << std : endl;
        std::cerr << "source: " << source << std : endl;
    }
#endif

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
        std::cerr << "FAILED TO COMPILE SHADER! type: " << (int)type << std::endl // Todo: Maybe map shadertype to some string if you care
                  << message << std::endl
                  << std::endl
                  << source << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

void Shader::CreateProgram()
{
    unsigned int program = glCreateProgram();

    std::vector<unsigned int> shader_references;
    shader_references.reserve(m_programSources.size());

    for (size_t i = 0; i < m_programSources.size(); i++)
    {
        const auto &ps = m_programSources[i];
        unsigned int shader_ref = CompileShader(ps.type, ps.content);
        glAttachShader(program, shader_ref);
        shader_references.push_back(shader_ref);
    }

    GLCALL(glLinkProgram(program));
    GLCALL(glValidateProgram(program));

    for (auto &&ref : shader_references)
    {
        GLCALL(glDeleteShader(ref));
    }

    m_RendererID = program;
}

Shader::Shader()
    : m_RendererID(0) {}

void Shader::addShader(const std::string &path, ShaderType type)
{
    m_programSources.push_back(ParseShader(path, type));
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

void Shader::SetUniform1f(const std::string &name, float f)
{
    GLCALL(glUniform1f(GetUniformLocation(name), f));
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
