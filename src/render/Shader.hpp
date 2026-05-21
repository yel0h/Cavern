#ifndef CAVERN_SHADER_HPP
#define CAVERN_SHADER_HPP
#include <glad/glad.h>
#include <string_view>

class Shader
{
private:
    static unsigned int compile(unsigned int type, std::string_view src);

public:
    unsigned int id = 0;

    void build(std::string_view vertSrc, std::string_view fragSrc);

    void use() const { glUseProgram(id); }

    void setMat4(const char *name, const float *v) const;

    void setInt(const char *name, int v) const { glUniform1i(glGetUniformLocation(id, name), v); }

    void setFloat(const char *name, float v) const { glUniform1f(glGetUniformLocation(id, name), v); }

    void setVec3(const char *name, float x, float y, float z) const;
};
#endif//CAVERN_SHADER_HPP