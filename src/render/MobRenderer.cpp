#include "MobRenderer.hpp"
#include "../entity/MobManager.hpp"
#include "Camera.hpp"
#include <algorithm>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

void MobRenderer::init()
{
    shader.build(vertSrc, fragSrc);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 27648 * sizeof(MobVertex), nullptr, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MobVertex), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MobVertex), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(MobVertex), (void *)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void MobRenderer::shutdown()
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

void MobRenderer::addBox(float x0, float y0, float z0, float x1, float y1, float z1, float br, float bg, float bb, float light, float yawDeg, float wx, float wy, float wz, float pvtX, float pvtY, float pvtZ, float rotAngle)
{
    struct Face
    {
        float c[4][3];
        float bright;
    };

    const Face faces[6] = {
            {{{x0, y1, z0}, {x1, y1, z0}, {x1, y1, z1}, {x0, y1, z1}}, 0.118f},
            {{{x0, y0, z1}, {x1, y0, z1}, {x1, y0, z0}, {x0, y0, z0}}, -0.118f},
            {{{x1, y0, z0}, {x1, y1, z0}, {x0, y1, z0}, {x0, y0, z0}}, 0.f},
            {{{x0, y0, z1}, {x0, y1, z1}, {x1, y1, z1}, {x1, y0, z1}}, 0.f},
            {{{x0, y0, z0}, {x0, y1, z0}, {x0, y1, z1}, {x0, y0, z1}}, 0.f},
            {{{x1, y0, z1}, {x1, y1, z1}, {x1, y1, z0}, {x1, y0, z0}}, 0.f},
    };
    float cy = std::cos(glm::radians(yawDeg));
    float sy = std::sin(glm::radians(yawDeg));
    float cr = std::cos(rotAngle);
    float sr = std::sin(rotAngle);
    auto transform = [&](float lx, float ly, float lz, MobVertex &v)
    {
        float dx = lx - pvtX;
        float dy = ly - pvtY;
        float dz = lz - pvtZ;
        float ry = (dy * cr) - (dz * sr);
        float rz = (dy * sr) + (dz * cr);
        float rx = dx;
        lx = rx + pvtX;
        ly = ry + pvtY;
        lz = rz + pvtZ;
        float fx = (lx * cy) - (lz * sy);
        float fz = (lx * sy) + (lz * cy);
        v.x = fx + wx;
        v.y = ly + wy;
        v.z = fz + wz;
    };
    auto clamp01 = [](float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); };
    for (const auto &face : faces)
    {
        float fr = clamp01(br + face.bright);
        float fg = clamp01(bg + face.bright);
        float fb = clamp01(bb + face.bright);
        int order[6] = {0, 2, 1, 0, 3, 2};
        for (int i : order)
        {
            MobVertex v{};
            transform(face.c[i][0], face.c[i][1], face.c[i][2], v);
            v.r = fr;
            v.g = fg;
            v.b = fb;
            v.light = light;
            verts.push_back(v);
        }
    }
}

void MobRenderer::buildSnout(float wx, float wy, float wz, float yawDeg, float legA, float legB, float light)
{
    constexpr float bodyR = 0.62f;
    constexpr float bodyG = 0.42f;
    constexpr float bodyB = 0.40f;
    constexpr float headR = 0.68f;
    constexpr float headG = 0.48f;
    constexpr float headB = 0.46f;
    addBox(-0.17f, 0.0f, -0.17f, -0.10f, 0.14f, -0.10f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, -0.13f, 0.14f, -0.13f, legA);
    addBox(0.10f, 0.0f, -0.17f, 0.17f, 0.14f, -0.10f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, 0.13f, 0.14f, -0.13f, -legA);
    addBox(-0.17f, 0.14f, -0.20f, 0.17f, 0.30f, 0.20f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, 0, 0, 0, 0.f);
    addBox(-0.17f, 0.0f, 0.10f, -0.10f, 0.14f, 0.17f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, -0.13f, 0.14f, 0.13f, legB);
    addBox(0.10f, 0.0f, 0.10f, 0.17f, 0.14f, 0.17f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, 0.13f, 0.14f, 0.13f, -legB);
    addBox(-0.12f, 0.12f, -0.36f, 0.12f, 0.27f, -0.20f, headR, headG, headB, light, yawDeg, wx, wy, wz, 0, 0, 0, 0.f);
}

void MobRenderer::buildBoneshade(float wx, float wy, float wz, float yawDeg, float armA, float legA, float light)
{
    constexpr float bodyR = 0.78f;
    constexpr float bodyG = 0.76f;
    constexpr float bodyB = 0.68f;
    constexpr float headR = 0.85f;
    constexpr float headG = 0.83f;
    constexpr float headB = 0.74f;
    addBox(-0.16f, 0.0f, -0.08f, -0.04f, 0.9f, 0.08f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, -0.10f, 0.9f, 0.f, legA);
    addBox(0.04f, 0.0f, -0.08f, 0.16f, 0.9f, 0.08f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, 0.10f, 0.9f, 0.f, -legA);
    addBox(-0.20f, 0.9f, -0.11f, 0.20f, 1.55f, 0.11f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, 0, 0, 0, 0.f);
    addBox(-0.30f, 0.95f, -0.08f, -0.20f, 1.5f, 0.08f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, -0.25f, 1.5f, 0.f, -armA);
    addBox(0.20f, 0.95f, -0.08f, 0.30f, 1.5f, 0.08f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, 0.25f, 1.5f, 0.f, armA);
    addBox(-0.14f, 1.55f, -0.14f, 0.14f, 1.85f, 0.14f, headR, headG, headB, light, yawDeg, wx, wy, wz, 0, 0, 0, 0.f);
}

void MobRenderer::buildGrubbin(float wx, float wy, float wz, float yawDeg, float armA, float legA, float light)
{
    constexpr float bodyR = 0.30f;
    constexpr float bodyG = 0.36f;
    constexpr float bodyB = 0.16f;
    constexpr float headR = 0.34f;
    constexpr float headG = 0.40f;
    constexpr float headB = 0.20f;
    addBox(-0.16f, 0.0f, -0.08f, -0.04f, 0.85f, 0.08f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, -0.10f, 0.85f, 0.f, legA);
    addBox(0.04f, 0.0f, -0.08f, 0.16f, 0.85f, 0.08f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, 0.10f, 0.85f, 0.f, -legA);
    addBox(-0.21f, 0.82f, -0.13f, 0.21f, 1.48f, 0.13f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, 0, 0, 0, 0.f);
    addBox(-0.31f, 0.90f, -0.09f, -0.21f, 1.42f, 0.09f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, -0.26f, 1.42f, 0.f, -armA);
    addBox(0.21f, 0.90f, -0.09f, 0.31f, 1.42f, 0.09f, bodyR, bodyG, bodyB, light, yawDeg, wx, wy, wz, 0.26f, 1.42f, 0.f, armA);
    addBox(-0.15f, 1.48f, -0.15f, 0.15f, 1.78f, 0.15f, headR, headG, headB, light, yawDeg, wx, wy, wz, 0, 0, 0, 0.f);
}

void MobRenderer::buildFumewretch(float wx, float wy, float wz, float yawDeg, float bob, float flashAmt, float light)
{
    constexpr float baseR = 0.22f;
    constexpr float baseG = 0.30f;
    constexpr float baseB = 0.14f;
    float r = baseR + ((1.f - baseR) * flashAmt);
    float g = baseG + ((1.f - baseG) * flashAmt);
    float b = baseB + ((1.f - baseB) * flashAmt);
    float bobY = bob * 0.04f;
    addBox(-0.30f, 0.0f + bobY, -0.30f, 0.30f, 0.75f + bobY, 0.30f, r, g, b, light, yawDeg, wx, wy, wz, 0, 0, 0, 0.f);
    addBox(-0.20f, 0.75f + bobY, -0.20f, 0.20f, 1.05f + bobY, 0.20f, r, g, b, light, yawDeg, wx, wy, wz, 0, 0, 0, 0.f);
}

void MobRenderer::render(const MobManager &mgr, const Camera &cam, int winW, int winH, float time)
{
    verts.clear();
    for (const auto &m : mgr.mobs)
    {
        switch (m.type)
        {
            case MobType::Snout:
            {
                float a = std::sin((time * 1.2f) + m.frontLegPhase) * glm::radians(14.0f);
                float b = std::sin((time * 1.2f) + m.rearLegPhase) * glm::radians(14.0f);
                buildSnout(m.position.x, m.position.y, m.position.z, m.yaw, a, b, m.light);
                break;
            }

            case MobType::Boneshade:
            {
                float legA = std::sin((time * 2.2f) + m.frontLegPhase) * glm::radians(22.0f);
                buildBoneshade(m.position.x, m.position.y, m.position.z, m.yaw, legA, legA, m.light);
                break;
            }

            case MobType::Grubbin:
            {
                float legA = std::sin((time * 1.8f) + m.frontLegPhase) * glm::radians(20.0f);
                buildGrubbin(m.position.x, m.position.y, m.position.z, m.yaw, legA, legA, m.light);
                break;
            }

            case MobType::Fumewretch:
            {
                float bob = std::sin((time * 3.f) + m.frontLegPhase);
                float flashAmt = std::sin(std::clamp(m.flashT, 0.f, 1.f) * 3.14159f);
                buildFumewretch(m.position.x, m.position.y, m.position.z, m.yaw, bob, flashAmt, m.light);
                break;
            }
        }
    }

    if (verts.empty())
    {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(MobVertex)), verts.data(), GL_STREAM_DRAW);
    float aspect = (winH > 0) ? (float) winW / (float) winH : 1.f;
    glm::mat4 vp = cam.viewProjection(aspect);
    shader.use();
    shader.setMat4("uMVP", glm::value_ptr(vp));
    shader.setFloat("uFogNear", 48.f);
    shader.setFloat("uFogFar", 64.f);
    shader.setVec3("uFogColor", 0.53f, 0.81f, 0.98f);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, (int)verts.size());
    glBindVertexArray(0);
}