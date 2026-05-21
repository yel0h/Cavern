#include "WandererRenderer.hpp"
#include "../entity/WandererManager.hpp"
#include "Camera.hpp"
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

void WandererRenderer::init()
{
    shader.build(vertSrc, fragSrc);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 21600 * sizeof(MobVertex), nullptr, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MobVertex), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MobVertex), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(MobVertex), (void *)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void WandererRenderer::shutdown()
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

void WandererRenderer::addBox(float x0, float y0, float z0, float x1, float y1, float z1, float br, float bg, float bb, float light, float yawDeg, float wx, float wy, float wz, float pvtX, float pvtY, float rotAngle)
{
    struct Face
    {
        float c[4][3];
        float bright;
    };

    const Face faces[6] = {
            {{{x0, y1, z0}, {x0, y1, z1}, {x1, y1, z1}, {x1, y1, z0}},  0.118f},
            {{{x0, y0, z1}, {x0, y0, z0}, {x1, y0, z0}, {x1, y0, z1}}, -0.118f},
            {{{x0, y0, z1}, {x0, y1, z1}, {x0, y1, z0}, {x0, y0, z0}},  0.f},
            {{{x1, y0, z0}, {x1, y1, z0}, {x1, y1, z1}, {x1, y0, z1}},  0.f},
            {{{x0, y0, z0}, {x0, y1, z0}, {x1, y1, z0}, {x1, y0, z0}},  0.f},
            {{{x1, y0, z1}, {x1, y1, z1}, {x0, y1, z1}, {x0, y0, z1}},  0.f},
    };

    float cy = std::cos(glm::radians(yawDeg));
    float sy = std::sin(glm::radians(yawDeg));
    float cr = std::cos(rotAngle);
    float sr = std::sin(rotAngle);
    auto transform = [&](float lx, float ly, float lz, MobVertex &v)
    {
        float dx = lx - pvtX;
        float dy = ly - pvtY;
        float dz = lz - 0.0;
        float ry = (dy * cr) - (dz * sr);
        float rz = (dy * sr) + (dz * cr);
        float rx = dx;
        lx = rx + pvtX;
        ly = ry + pvtY;
        lz = rz + 0.0;
        float fx = (lx * cy) + (lz * sy);
        float fz = (-lx * sy) + (lz * cy);
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
        int order[6] = {0, 1, 2, 0, 2, 3};
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

void WandererRenderer::buildMobMesh(float wx, float wy, float wz, float yawDeg, float leftArmAngle, float rightArmAngle, float light)
{
    constexpr float limbR = 0.314f, limbG = 0.392f, limbB = 0.627f;
    constexpr float headR = 0.863f, headG = 0.706f, headB = 0.549f;
    addBox(-0.2f, 0.0f, -0.1f, 0.0f, 0.6f, 0.1f,
           limbR, limbG, limbB, light, yawDeg, wx, wy, wz, 0, 0, 0.f);
    addBox(0.0f, 0.0f, -0.1f, 0.2f, 0.6f, 0.1f,
           limbR, limbG, limbB, light, yawDeg, wx, wy, wz, 0, 0, 0.f);
    addBox(-0.2f, 0.6f, -0.1f, 0.2f, 1.3f, 0.1f,
           limbR, limbG, limbB, light, yawDeg, wx, wy, wz, 0, 0, 0.f);
    addBox(-0.4f, 0.7f, -0.1f, -0.2f, 1.3f, 0.1f,
           limbR, limbG, limbB, light, yawDeg, wx, wy, wz, -0.3f, 1.3f, leftArmAngle);
    addBox(0.2f, 0.7f, -0.1f, 0.4f, 1.3f, 0.1f,
           limbR, limbG, limbB, light, yawDeg, wx, wy, wz, 0.3f, 1.3f, rightArmAngle);
    addBox(-0.2f, 1.3f, -0.2f, 0.2f, 1.7f, 0.2f,
           headR, headG, headB, light, yawDeg, wx, wy, wz, 0, 0, 0.f);
}

void WandererRenderer::render(const WandererManager &mgr, const Camera &cam, int winW, int winH, float time)
{
    verts.clear();
    for (const auto &w : mgr.wanderers)
    {
        float leftAngle = std::sin((time * 2.0f) + w.leftArmPhase) * glm::radians(40.0f);
        float rightAngle = std::sin((time * 2.0f) + w.rightArmPhase) * glm::radians(40.0f);
        buildMobMesh(w.position.x, w.position.y, w.position.z,
                     w.yaw, leftAngle, rightAngle, w.light);
    }

    if (verts.empty())
    {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(MobVertex)), verts.data(), GL_STREAM_DRAW);
    float aspect = (winH > 0) ? (float)winW / (float)winH : 1.f;
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