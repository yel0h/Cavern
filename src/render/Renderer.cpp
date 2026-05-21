#include "Renderer.hpp"
#include "../world/World.hpp"
#include <glm/gtc/type_ptr.hpp>

static const int faceCorners[6][4][3] = {
        {{0, 1, 0}, {0, 1, 1}, {1, 1, 1}, {1, 1, 0}},
        {{0, 0, 1}, {0, 0, 0}, {1, 0, 0}, {1, 0, 1}},
        {{0, 0, 1}, {0, 1, 1}, {0, 1, 0}, {0, 0, 0}},
        {{1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 1}},
        {{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}},
        {{1, 0, 1}, {1, 1, 1}, {0, 1, 1}, {0, 0, 1}},
};

static const float faceNormals[6][3] = {
        {0.f, 1.f, 0.f},
        {0.f, -1.f, 0.f},
        {-1.f, 0.f, 0.f},
        {1.f, 0.f, 0.f},
        {0.f, 0.f, -1.f},
        {0.f, 0.f, 1.f},
};

void Renderer::init()
{
    shader.build(vertSrc, fragSrc);
    atlas.build();
    initHighlight();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void Renderer::shutdown()
{
    for (auto &m : meshes)
    {
        m.free();
        if (hlVao)
        {
            glDeleteVertexArrays(1, &hlVao);
            hlVao = 0;
        }

        if (hlVbo)
        {
            glDeleteBuffers(1, &hlVbo);
            hlVbo = 0;
        }
    }
}

void Renderer::initHighlight()
{
    hlShader.build(hlVertSrc, hlFragSrc);
    glGenVertexArrays(1, &hlVao);
    glGenBuffers(1, &hlVbo);
    glBindVertexArray(hlVao);
    glBindBuffer(GL_ARRAY_BUFFER, hlVbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void Renderer::renderHighlight(const Renderer::HighlightFace &hl, const float *vp, float time)
{
    if (!hl.valid)
    {
        return;
    }

    const auto &corners = faceCorners[hl.face];
    const float *norm = faceNormals[hl.face];
    constexpr float nudge = 0.002f;
    float verts[4][3];
    for (int i = 0; i < 4; i++)
    {
        verts[i][0] = (float)hl.bx + corners[i][0] + (norm[0] * nudge);
        verts[i][1] = (float)hl.by + corners[i][1] + (norm[1] * nudge);
        verts[i][2] = (float)hl.bz + corners[i][2] + (norm[2] * nudge);
    }

    glBindBuffer(GL_ARRAY_BUFFER, hlVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    hlShader.use();
    hlShader.setMat4("uMVP", vp);
    hlShader.setFloat("uTime", time);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(hlVao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
}

void Renderer::rebuildDirty(const World &world)
{
    for (int cz = 0; cz < World::CHUNKS_Z; cz++)
    {
        for (int cx = 0; cx < World::CHUNKS_X; cx++)
        {
            auto *chunk = const_cast<Chunk *>(world.getChunk(cx, cz));
            if (!chunk || !chunk->dirty)
            {
                continue;
            }

            int idx = (cz * World::CHUNKS_X) + cx;
            meshes[idx].build(*chunk, world, atlas);
            meshes[idx].upload();
            chunk->dirty = false;
        }
    }
}

void Renderer::renderFrame(const World &world, const Camera &cam, int winW, int winH, const Renderer::HighlightFace &hl, float time)
{
    rebuildDirty(world);
    glViewport(0, 0, winW, winH);
    glClearColor(0.53f, 0.81f, 0.98f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float aspect = (winH > 0) ? (float)winW / (float)winH : 1.f;
    glm::mat4 vp = cam.viewProjection(aspect);
    shader.use();
    shader.setMat4("uMVP", glm::value_ptr(vp));
    shader.setInt("uAtlas", 0);
    shader.setFloat("uFogNear", 48.f);
    shader.setFloat("uFogFar", 64.f);
    shader.setVec3("uFogColor", 0.53f, 0.81f, 0.98f);
    atlas.bind(0);
    for (int i = 0; i < 256; i++)
    {
        meshes[i].draw();
    }

    renderHighlight(hl, glm::value_ptr(vp), time);
}