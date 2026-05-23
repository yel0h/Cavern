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
    initCrosshair();
    initHUD();
    initText();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void Renderer::shutdown()
{
    for (auto &m : meshes)
    {
        m.free();
    }

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

    if (xhVao)
    {
        glDeleteVertexArrays(1, &xhVao);
        xhVao = 0;
    }

    if (xhVbo)
    {
        glDeleteBuffers(1, &xhVbo);
        xhVbo  = 0;
    }

    if (hudVao)
    {
        glDeleteVertexArrays(1, &hudVao);
        hudVao = 0;
    }

    if (hudVbo)
    {
        glDeleteBuffers(1, &hudVbo);
        hudVbo = 0;
    }

    if (txtVao)
    {
        glDeleteVertexArrays(1, &txtVao);
        txtVao = 0;
    }

    if (txtVbo)
    {
        glDeleteBuffers(1, &txtVbo);
        txtVbo = 0;
    }

    font.shutdown();
}

void Renderer::initHighlight()
{
    hlShader.build(hlVertSrc, hlFragSrc);
    glGenVertexArrays(1, &hlVao);
    glGenBuffers(1, &hlVbo);
    glBindVertexArray(hlVao);
    glBindBuffer(GL_ARRAY_BUFFER, hlVbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void Renderer::renderHighlight(const Renderer::HighlightBlock &hl, const float *vp, float time)
{
    if (!hl.valid)
    {
        return;
    }

    constexpr float kNudge = 0.003f;
    float verts[6][4][3];
    for (int f = 0; f < 6; f++)
    {
        const auto &corners = faceCorners[f];
        const float *norm = faceNormals[f];
        for (int i = 0; i < 4; i++)
        {
            verts[f][i][0] = (float)hl.bx + corners[i][0] + (norm[0] * kNudge);
            verts[f][i][1] = (float)hl.by + corners[i][1] + (norm[1] * kNudge);
            verts[f][i][2] = (float)hl.bz + corners[i][2] + (norm[2] * kNudge);
        }
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
    for (int f = 0; f < 6; f++)
    {
        glDrawArrays(GL_TRIANGLE_FAN, f * 4, 4);
    }

    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
}

void Renderer::initCrosshair()
{
    _2dShader.build(crosshairVertSrc, crosshairFragSrc);
    glGenVertexArrays(1, &xhVao);
    glGenBuffers(1, &xhVbo);
    glBindVertexArray(xhVao);
    glBindBuffer(GL_ARRAY_BUFFER, xhVbo);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void Renderer::renderCrosshair(int winW, int winH)
{
    float nx = 10.f / (float)winW;
    float ny = 10.f / (float)winH;
    float tx = 2.f / (float)winW;
    float ty = 2.f / (float)winH;
    float verts[24] = {
            -nx, -ty, nx, -ty, nx, ty,
            -nx, -ty, nx, ty, -nx, ty,
            -tx, -ny, tx, -ny, tx, ny,
            -tx, -ny, tx, ny, -tx, ny,
    };
    glBindBuffer(GL_ARRAY_BUFFER, xhVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    _2dShader.use();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(xhVao);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Renderer::initHUD()
{
    hudShader.build(hudVertSrc, hudFragSrc);
    glGenVertexArrays(1, &hudVao);
    glGenBuffers(1, &hudVbo);
    glBindVertexArray(hudVao);
    glBindBuffer(GL_ARRAY_BUFFER, hudVbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void Renderer::renderHUD(int winW, int winH, BlockType selectedBlock)
{
    float qW = 64.f;
    float qH = 64.f;
    float inset = 8.f;
    float x1 = 1.f - ((inset / (float)winW) * 2.f);
    float x0 = x1 - ((qW / (float)winW) * 2.f);
    float y1 = 1.f - ((inset / (float)winH) * 2.f);
    float y0 = y1 - ((qH / (float)winH) * 2.f);
    unsigned char tileIdx = blockDef(selectedBlock).texTop;
    float u0;
    float v0;
    float u1;
    float v1;
    TextureAtlas::uvRect(tileIdx, u0, v0, u1, v1);
    float verts[6 * 4] = {
            x0, y0, u1, v1,
            x1, y0, u0, v1,
            x1, y1, u0, v0,
            x0, y0, u1, v1,
            x1, y1, u0, v0,
            x0, y1, u1, v0,
    };
    glBindBuffer(GL_ARRAY_BUFFER, hudVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    hudShader.use();
    hudShader.setInt("uAtlas", 0);
    atlas.bind(0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(hudVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Renderer::initText()
{
    txtShader.build(txtVertSrc, txtFragSrc);
    font.init();
    glGenVertexArrays(1, &txtVao);
    glGenBuffers(1, &txtVbo);
    glBindVertexArray(txtVao);
    glBindBuffer(GL_ARRAY_BUFFER, txtVbo);
    glBufferData(GL_ARRAY_BUFFER, 128 * 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void Renderer::drawText(const char *text, float px, float py, int scale, int winW, int winH)
{
    float fw = winW;
    float fh = winH;
    float cw = Font::CHAR_W * scale;
    float ch = Font::CHAR_H * scale;
    std::vector<float> verts;
    verts.reserve(64 * 6 * 4);
    float cx = px;
    for (const char *p = text; *p; p++)
    {
        float u0;
        float v0;
        float u1;
        float v1;
        font.uvForChar(*p, u0, v0, u1, v1);
        float x0 = (cx / fw * 2.f) - 1.f;
        float x1 = ((cx + cw) / fw * 2.f) - 1.f;
        float y1 = 1.f - (py / fh * 2.f);
        float y0 = 1.f - ((py + ch) / fh * 2.f);
        verts.insert(verts.end(), {x0, y0, u0, v0, x1, y0, u1, v0, x1, y1, u1, v1});
        verts.insert(verts.end(), {x0, y0, u0, v0, x1, y1, u1, v1, x0, y1, u0, v1});
        cx += cw;
    }

    if (verts.empty())
    {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, txtVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(float), verts.data());
    glBindVertexArray(txtVao);
    glDrawArrays(GL_TRIANGLES, 0, verts.size() / 4);
    glBindVertexArray(0);
}

void Renderer::renderDebug(int winW, int winH, int fps, int chunkUpdates, bool placeMode)
{
    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    txtShader.setVec3("uColor", 1.f, 1.f, 1.f);
    constexpr int verScale = 3;
    drawText(version, 4.f, 4.f, verScale, winW, winH);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "FPS: %d  CHUNKS: %d", fps, chunkUpdates);
    float statsY = 4.f + (Font::CHAR_H * verScale) + 4.f;
    drawText(buf, 4.f, statsY, 1, winW, winH);
    constexpr int modeScale = 2;
    const char *modeLabel = placeMode ? "PLACE" : "DIG";
    float modeW = std::strlen(modeLabel) * Font::CHAR_W * modeScale;
    drawText(modeLabel, (float)winW - modeW - 4.f, 4.f, modeScale, winW, winH);
    glEnable(GL_DEPTH_TEST);
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
            lastChunkUpdates++;
        }
    }
}

void Renderer::renderFrame(const World &world, const Camera &cam, int winW, int winH, const Renderer::HighlightBlock &hl, float time, BlockType selectedBlock, int fps, int chunkUpdates, bool placeMode)
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
    renderCrosshair(winW, winH);
    renderHUD(winW, winH, selectedBlock);
    renderDebug(winW, winH, fps, chunkUpdates, placeMode);
}