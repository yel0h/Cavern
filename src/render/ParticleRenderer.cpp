#include "ParticleRenderer.hpp"
#include "../entity/ParticleManager.hpp"
#include <glm/gtc/type_ptr.hpp>

void ParticleRenderer::init()
{
    shader.build(vertSrc, fragSrc);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);
    glEnable(GL_PROGRAM_POINT_SIZE);
}

void ParticleRenderer::shutdown()
{
    if (vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    if (vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
}

void ParticleRenderer::render(const std::vector<BlockParticle> &particles, const glm::mat4 &vp)
{
    if (particles.empty())
    {
        return;
    }

    std::vector<float> data;
    data.reserve(particles.size() * 6);
    for (const auto &p : particles)
    {
        data.push_back(p.pos.x);
        data.push_back(p.pos.y);
        data.push_back(p.pos.z);
        data.push_back(p.r);
        data.push_back(p.g);
        data.push_back(p.b);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(data.size() * sizeof(float)), data.data(), GL_STREAM_DRAW);
    shader.use();
    shader.setMat4("uMVP", glm::value_ptr(vp));
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, (int)particles.size());
    glBindVertexArray(0);
}