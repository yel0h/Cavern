#include "ParticleRenderer.hpp"
#include "../entity/ParticleManager.hpp"
#include "../entity/ItemDrop.hpp"
#include "../entity/Bolt.hpp"
#include <glm/gtc/type_ptr.hpp>

void ParticleRenderer::init()
{
    shader.build(vertSrc, fragSrc);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
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
    shader.setFloat("uPointSize", 4.0f);
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, (int)particles.size());
    glBindVertexArray(0);
}

void ParticleRenderer::renderDrops(const std::vector<ItemDrop> &drops, const glm::mat4 &vp)
{
    if (drops.empty())
    {
        return;
    }

    std::vector<float> data;
    data.reserve(drops.size() * 6);
    for (const auto &d : drops)
    {
        data.push_back(d.position.x);
        data.push_back(d.position.y);
        data.push_back(d.position.z);
        data.push_back(d.r);
        data.push_back(d.g);
        data.push_back(d.b);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (data.size() * sizeof(float)), data.data(), GL_STREAM_DRAW);
    shader.use();
    shader.setMat4("uMVP", glm::value_ptr(vp));
    shader.setFloat("uPointSize", 9.0f);
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, (GLsizei) drops.size());
    glBindVertexArray(0);
}

void ParticleRenderer::renderBolts(const std::vector<Bolt> &bolts, const glm::mat4 &vp)
{
    if (bolts.empty())
    {
        return;
    }

    std::vector<float> data;
    data.reserve(bolts.size() * 6);
    for (const auto &b: bolts)
    {
        data.push_back(b.position.x);
        data.push_back(b.position.y);
        data.push_back(b.position.z);
        data.push_back(0.85f);
        data.push_back(0.80f);
        data.push_back(0.30f);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(data.size() * sizeof(float)), data.data(), GL_STREAM_DRAW);
    shader.use();
    shader.setMat4("uMVP", glm::value_ptr(vp));
    shader.setFloat("uPointSize", 6.0f);
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, (int)bolts.size());
    glBindVertexArray(0);
}