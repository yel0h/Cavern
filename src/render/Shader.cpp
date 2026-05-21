#include "Shader.hpp"
#include <stdexcept>
#include <vector>

unsigned int Shader::compile(unsigned int type, std::string_view src)
{
    unsigned int s = glCreateShader(type);
    const char *p = src.data();
    int len = (int)src.size();
    glShaderSource(s, 1, &p, &len);
    glCompileShader(s);
    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        int logLen;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetShaderInfoLog(s, logLen, nullptr, log.data());
        glDeleteShader(s);
        throw std::runtime_error(std::string("Shader compile error: ") + log.data());
    }

    return s;
}

void Shader::build(std::string_view vertSrc, std::string_view fragSrc)
{
    unsigned int vs = compile(GL_VERTEX_SHADER, vertSrc);
    unsigned int fs = compile(GL_FRAGMENT_SHADER, fragSrc);
    id = glCreateProgram();
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);
    glDeleteShader(vs);
    glDeleteShader(fs);
    int ok;
    glGetProgramiv(id, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        int logLen;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetProgramInfoLog(id, logLen, nullptr, log.data());
        glDeleteProgram(id);
        throw std::runtime_error(std::string("Shader link error: ") + log.data());
    }
}

void Shader::setMat4(const char *name, const float *v) const
{
    glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, v);
}

void Shader::setVec3(const char *name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(id, name), x, y, z);
}