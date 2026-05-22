#ifndef CAVERN_PARTICLERENDERER_HPP
#define CAVERN_PARTICLERENDERER_HPP
#include "Shader.hpp"
#include <glm/glm.hpp>
#include <vector>

struct BlockParticle;

class ParticleRenderer
{
private:
    Shader shader;
    unsigned int vao = 0;
    unsigned int vbo = 0;
    static constexpr const char *vertSrc = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;

uniform mat4 uMVP;

out vec3 vColor;

void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
    gl_PointSize = 4.0;
    vColor = aColor;
}
)";
    static constexpr const char *fragSrc = R"(
#version 330 core
in vec3 vColor;

out vec4 fragColor;

void main()
{
    fragColor = vec4(vColor, 1.0);
}
)";

public:
    void init();

    void shutdown();

    void render(const std::vector<BlockParticle> &particles, const glm::mat4 &vp);
};
#endif//CAVERN_PARTICLERENDERER_HPP