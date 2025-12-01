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
        std::cerr << "Could not create shader with type: " << (int)type << std::endl;
        std::cerr << "shader_str: " << shader_str << std::endl;
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
    size_t i = 0;
    for (i; i < m_programSources.size(); i++)
    {
        const auto &ps = m_programSources[i];
        unsigned int shader_ref = compileShader(ps.type, ps.content);
        if (shader_ref == 0)
        {
            GLCALL(glDeleteProgram(program));
            exit(-1);
        }
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

void Shader::addShader(const std::string &filename_nopath, ShaderType type)
{
    fs::path directory;
    if (type == ShaderType::VERTEX)
        directory = VERTEX_SHADER_DIR;
    else if (type == ShaderType::FRAGMENT)
        directory = FRAGMENT_SHADER_DIR;
    else
    {
        std::cerr << "Error: Unsupported shader type for automatic directory selection." << std::endl;
        return;
    }

    m_programSources.push_back(parseShader((directory / filename_nopath).string(), type));
}

Shader::~Shader()
{
#ifdef DEBUG
    DEBUG_PRINT("Deleting shader program with ID: " << m_RendererID);
#endif
    GLCALL(glDeleteProgram(m_RendererID));
}

void Shader::bind() const
{
    GLCALL(glUseProgram(m_RendererID));
    RenderingContext *rContext = RenderingContext::Current();
    rContext->m_boundShader = m_RendererID;
}

void Shader::unbind() const
{
    GLCALL(glUseProgram(0));
    RenderingContext *rContext = RenderingContext::Current();
    rContext->m_boundShader = 0;
}

bool Shader::checkAndUpdateUVCache(const std::string &name, const UniformValue &value)
{
    auto it = m_UniformValueCache.find(name);
    if (it == m_UniformValueCache.end())
        return false;

    if(it->second == value) // variant knows how to compare
        return true;
    else{
        m_UniformValueCache[name] = value;
        return false;
    }
}

void Shader::setUniform(const std::string &name, int v)
{
    if (checkAndUpdateUVCache(name, v))
        return;
    GLCALL(glUniform1i(getUniformLocation(name), v));
}

void Shader::setUniform(const std::string &name, unsigned int v)
{
    if (checkAndUpdateUVCache(name, v))
        return;
    GLCALL(glUniform1i(getUniformLocation(name), v));    
}

// Float/double uniforms
void Shader::setUniform(const std::string &name, float v)
{
    if (checkAndUpdateUVCache(name, v))
        return;
    GLCALL(glUniform1f(getUniformLocation(name), v));    
}

// Float vector uniforms
void Shader::setUniform(const std::string &name, const glm::vec2 &v)
{
    if (checkAndUpdateUVCache(name, v))
        return;
    GLCALL(glUniform2fv(getUniformLocation(name), 1, &v[0]));    
}
void Shader::setUniform(const std::string &name, const glm::vec3 &v)
{
    if (checkAndUpdateUVCache(name, v))
        return;
    GLCALL(glUniform3fv(getUniformLocation(name), 1, &v[0]));    
}
void Shader::setUniform(const std::string &name, const glm::vec4 &v)
{
    if (checkAndUpdateUVCache(name, v))
        return;
    GLCALL(glUniform4fv(getUniformLocation(name), 1, &v[0]));    
}

// Matrix uniforms
void Shader::setUniform(const std::string &name, const glm::mat4 &v)
{
    if (checkAndUpdateUVCache(name, v))
        return;
    GLCALL(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(v)));
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
