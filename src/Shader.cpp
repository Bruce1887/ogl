#include "Shader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <cassert>

ShaderProgramSource Shader::parseShader(const std::string &filepath, ShaderType type)
{
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file)
    {
        std::cerr << "Error: Failed to open shader file: " << filepath << std::endl;
        return {"", ShaderType::UNASSIGNED};
    }

    std::string source;
    file.seekg(0, std::ios::end);
    source.reserve(static_cast<size_t>(file.tellg()));
    file.seekg(0, std::ios::beg);

    source.assign((std::istreambuf_iterator<char>(file)),
                  std::istreambuf_iterator<char>());

    return {std::move(source), type};
}

unsigned int Shader::compileShader(ShaderType type, const std::string &shader_str)
{
    unsigned int id = glCreateShader((int)type);
#ifdef DEBUG
    if (id == 0)
    {
        std::cerr << "Could not create shader with type: " << type << std : endl;
        std::cerr << "shader_str: " << shader_str << std : endl;
    }
#endif

    const char *shader_c_str = shader_str.c_str();
    glShaderSource(id, 1, &shader_c_str, nullptr);
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
                  << shader_str << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

void Shader::createProgram()
{
    unsigned int program = glCreateProgram();

    std::vector<unsigned int> shader_references;
    shader_references.reserve(m_programSources.size());

    for (size_t i = 0; i < m_programSources.size(); i++)
    {
        const auto &ps = m_programSources[i];
        unsigned int shader_ref = compileShader(ps.type, ps.content);
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
    m_programSources.push_back(parseShader(path, type));
}

Shader::~Shader()
{
    GLCALL(glDeleteProgram(m_RendererID));
}

void Shader::bind() const
{
    GLCALL(glUseProgram(m_RendererID));
}

void Shader::unbind() const
{
    GLCALL(glUseProgram(0));
}

void Shader::setUniform(const std::string &name, int value) {
    GLCALL(glUniform1i(getUniformLocation(name), value));
}

// Float/double uniforms
void Shader::setUniform(const std::string &name, float value) {
    GLCALL(glUniform1f(getUniformLocation(name), value));
}

// Float vector uniforms
void Shader::setUniform(const std::string &name, const glm::vec2 &v) {
    GLCALL(glUniform2fv(getUniformLocation(name), 1, &v[0]));   
}
void Shader::setUniform(const std::string &name, const glm::vec3 &v) {
    GLCALL(glUniform3fv(getUniformLocation(name), 1, &v[0]));
}
void Shader::setUniform(const std::string &name, const glm::vec4 &v) {
    GLCALL(glUniform4fv(getUniformLocation(name), 1, &v[0]));
}

// Matrix uniforms
void Shader::setUniform(const std::string &name, const glm::mat4 &m) {
    GLCALL(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(m)));

}

int Shader::getUniformLocation(const std::string &name)
{
    // Check cache first
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
    {
        return m_UniformLocationCache[name];
    }

    // Not found in cache, query OpenGL
    GLCALL(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if (location == -1)
    {
        std::cerr << "Uniform " << name << " not found!" << std::endl;
    }
    
    // Cache the location
    m_UniformLocationCache[name] = location;
    return location;
}
