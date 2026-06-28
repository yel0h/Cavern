#include "Renderer.hpp"
#include "../world/World.hpp"
#include <glm/gtc/type_ptr.hpp>

static std::array<glm::vec4, 6> extractFrustumPlanes(const glm::mat4 &vp)
{
    auto row = [&](int r)
    {
        return glm::vec4(vp[0][r], vp[1][r], vp[2][r], vp[3][r]);
    };
    return {
            row(3) + row(0),
            row(3) - row(0),
            row(3) + row(1),
            row(3) - row(1),
            row(3) + row(2),
            row(3) - row(2),
    };
}

static bool chunkInFrustum(int cx, int cz, const std::array<glm::vec4, 6> &planes)
{
    auto x0 = (float)(cx * 16);
    float x1 = x0 + 16.f;
    float y0 = 0.f;
    float y1 = 64.f;
    auto z0 = (float)(cz * 16);
    float z1 = z0 + 16.f;
    return std::ranges::all_of(planes, [x1, x0, y1, y0, z1, z0](const auto &p)
                               {
                                    float px = (p.x >= 0.f) ? x1 : x0;
                                    float py = (p.y >= 0.f) ? y1 : y0;
                                    float pz = (p.z >= 0.f) ? z1 : z0;
                                    return (p.x * px) + (p.y * py) + (p.z * pz) + p.w >= 0.f;
                                });
}

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
    initOutline();
    initCrosshair();
    initHUD();
    initIconAtlas();
    initText();
    initClouds();
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

    if (olVao)
    {
        glDeleteVertexArrays(1, &olVao);
        olVao = 0;
    }

    if (olVbo)
    {
        glDeleteBuffers(1, &olVbo);
        olVbo = 0;
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

    if (cloudVao)
    {
        glDeleteVertexArrays(1, &cloudVao);
        cloudVao = 0;
    }

    if (cloudVbo)
    {
        glDeleteBuffers(1, &cloudVbo);
        cloudVbo = 0;
    }

    if (iconAtlas)
    {
        glDeleteTextures(1, &iconAtlas);
        iconAtlas = 0;
    }

    if (fullIconAtlas)
    {
        glDeleteTextures(1, &fullIconAtlas);
        fullIconAtlas = 0;
    }

    font.shutdown();
}

static unsigned int cloudHash(int gx, int gz)
{
    unsigned int h = (unsigned)(gx * 2654435761u) ^ (unsigned)(gz * 805459861u) ^ 0xC10CD5u;
    h ^= h >> 16;
    h *= 0x45d9f3bu;
    h ^= h >> 16;
    return h;
}

void Renderer::initClouds()
{
    cloudShader.build(cloudVertSrc, cloudFragSrc);
    static constexpr int cells = 16;
    static constexpr float cell = 16.f;
    static constexpr float y = 66.f;
    std::vector<float> verts;
    verts.reserve(cells * cells * 6 * 3);
    for (int gz = 0; gz < cells; gz++)
    {
        for (int gx = 0; gx < cells; gx++)
        {
            if ((cloudHash(gx, gz) & 0xFFFF) < 0x8000)
            {
                continue;
            }

            float x0 = (float)gx * cell;
            float x1 = x0 + cell;
            float z0 = (float)gz * cell;
            float z1 = z0 + cell;
            verts.insert(verts.end(), {x0, y, z0, x1, y, z0, x1, y, z1});
            verts.insert(verts.end(), {x0, y, z0, x1, y, z1, x0, y, z1});
        }
    }

    cloudVertCount = (int)verts.size() / 3;
    glGenVertexArrays(1, &cloudVao);
    glGenBuffers(1, &cloudVbo);
    glBindVertexArray(cloudVao);
    glBindBuffer(GL_ARRAY_BUFFER, cloudVbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(float)), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void Renderer::renderClouds(const float *vp, float time)
{
    if (cloudVertCount == 0)
    {
        return;
    }

    float drift = std::fmod(time * 1.5f, 256.f);
    cloudShader.use();
    glUniformMatrix4fv(glGetUniformLocation(cloudShader.id, "uVP"), 1, GL_FALSE, vp);
    glUniform1f(glGetUniformLocation(cloudShader.id, "uDrift"), drift);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(cloudVao);
    glDrawArrays(GL_TRIANGLES, 0, cloudVertCount);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
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

void Renderer::renderHighlight(const Renderer::HighlightBlock &hl, const float *vp, float time, const World &world)
{
    if (!hl.valid)
    {
        return;
    }

    BlockType hlBlock = world.getBlock(hl.bx, hl.by, hl.bz);
    if (blockDef(hlBlock).transparent && !blockDef(hlBlock).liquid && world.getLight(hl.bx, hl.by, hl.bz) == 0)
    {
        return;
    }

    float verts[6][4][3];
    for (int f = 0; f < 6; f++)
    {
        const auto &corners = faceCorners[f];
        for (int i = 0; i < 4; i++)
        {
            verts[f][i][0] = (float)hl.bx + corners[i][0];
            verts[f][i][1] = (float)hl.by + corners[i][1];
            verts[f][i][2] = (float)hl.bz + corners[i][2];
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, hlVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    hlShader.use();
    hlShader.setMat4("uMVP", vp);
    hlShader.setFloat("uTime", time);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    glBindVertexArray(hlVao);
    for (int f = 0; f < 6; f++)
    {
        glDrawArrays(GL_TRIANGLE_FAN, f * 4, 4);
    }

    glBindVertexArray(0);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_BLEND);
}

void Renderer::initOutline()
{
    olShader.build(hlVertSrc, olFragSrc);
    glGenVertexArrays(1, &olVao);
    glGenBuffers(1, &olVbo);
    glBindVertexArray(olVao);
    glBindBuffer(GL_ARRAY_BUFFER, olVbo);
    glBufferData(GL_ARRAY_BUFFER, 24 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void Renderer::renderOutline(const Renderer::HighlightBlock &hl, const float *vp)
{
    if (!hl.valid)
    {
        return;
    }

    auto x0 = (float)hl.bx;
    auto y0 = (float)hl.by;
    auto z0 = (float)hl.bz;
    float x1 = (float)hl.bx + 1.f;
    float y1 = (float)hl.by + 1.f;
    float z1 = (float)hl.bz + 1.f;
    float verts[24][3] = {
            {x0, y0, z0}, {x1, y0, z0},
            {x1, y0, z0}, {x1, y0, z1},
            {x1, y0, z1}, {x0, y0, z1},
            {x0, y0, z1}, {x0, y0, z0},
            {x0, y1, z0}, {x1, y1, z0},
            {x1, y1, z0}, {x1, y1, z1},
            {x1, y1, z1}, {x0, y1, z1},
            {x0, y1, z1}, {x0, y1, z0},
            {x0, y0, z0}, {x0, y1, z0},
            {x1, y0, z0}, {x1, y1, z0},
            {x1, y0, z1}, {x1, y1, z1},
            {x0, y0, z1}, {x0, y1, z1},
    };

    glBindBuffer(GL_ARRAY_BUFFER, olVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    olShader.use();
    olShader.setMat4("uMVP", vp);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(olVao);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
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
    glBufferData(GL_ARRAY_BUFFER, 64 * 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void const *>(2 * sizeof(float)));
    glBindVertexArray(0);
}

void Renderer::initIconAtlas()
{
    static constexpr BlockType hotbar[] = {
            BlockType::Pith, BlockType::Stone, BlockType::Soil, BlockType::Boards,
            BlockType::Sapling, BlockType::Timber, BlockType::Glaze, BlockType::Grit
    };
    static constexpr int size = 8;
    int tops[size];
    int sides[size];
    for (int i = 0; i < size; i++)
    {
        BlockType bt = hotbar[i];
        if (bt == BlockType::Water || bt == BlockType::Lava)
        {
            tops[i] = -1;
            sides[i] = -1;
        }
        else
        {
            tops[i] = (int)blockDef(bt).texTop;
            sides[i] = (int)blockDef(bt).texSide;
        }
    }

    iconAtlas = atlas.buildIconAtlas(tops, sides, size);
}

void Renderer::renderHUD(int winW, int winH, int hotbarSize, int hotbarIdx)
{
    static constexpr float slot = 40.f;
    static constexpr float gap = 2.f;
    static constexpr float inset = 8.f;
    auto fw = (float)winW;
    auto fh = (float)winH;
    float barW = (hotbarSize * slot) + ((hotbarSize - 1) * gap);
    float barX0 = (fw - barW) * 0.5f;
    float barY0 = inset;
    auto toNDC = [&](float px, float py) -> std::pair<float,float>
    {
        return {(px / fw * 2.f) - 1.f, (py / fh * 2.f) - 1.f};
    };
    auto pushQuad = [](std::vector<float> &v,
                       float x0, float y0, float x1, float y1,
                       float u0, float vv0, float u1, float vv1)
    {
        v.insert(v.end(), {
                                  x0, y0, u0, vv1, x1, y0, u1, vv1, x1, y1, u1, vv0,
                                  x0, y0, u0, vv1, x1, y1, u1, vv0, x0, y1, u0, vv0,
                          });
    };

    std::vector<float> iconVerts;
    std::vector<float> bgVerts;
    for (int i = 0; i < hotbarSize; i++)
    {
        float sx0 = barX0 + (i * (slot + gap));
        float sx1 = sx0 + slot;
        float sy0 = barY0;
        float sy1 = sy0 + slot;
        auto [nx0, ny0] = toNDC(sx0, sy0);
        auto [nx1, ny1] = toNDC(sx1, sy1);
        float au0;
        float av0;
        float au1;
        float av1;
        TextureAtlas::uvRect(4, au0, av0, au1, av1);
        pushQuad(bgVerts, nx0, ny0, nx1, ny1, au0, av0, au1, av1);
        float iu0 = (float)i / (float)hotbarSize;
        float iu1 = (float)(i + 1) / (float)hotbarSize;
        pushQuad(iconVerts, nx0, ny0, nx1, ny1, iu0, 0.f, iu1, 1.f);
    }

    glBindBuffer(GL_ARRAY_BUFFER, hudVbo);
    glBindVertexArray(hudVao);
    hudShader.use();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    hudShader.setInt("uAtlas", 0);
    atlas.bind(0);
    glUniform4f(glGetUniformLocation(hudShader.id, "uTint"), 0.f, 0.f, 0.f, 0.55f);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(bgVerts.size() * sizeof(float)), bgVerts.data());
    glDrawArrays(GL_TRIANGLES, 0, (int)(bgVerts.size() / 4));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, iconAtlas);
    glUniform4f(glGetUniformLocation(hudShader.id, "uTint"), 1.f, 1.f, 1.f, 1.f);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(iconVerts.size() * sizeof(float)), iconVerts.data());
    glDrawArrays(GL_TRIANGLES, 0, (int)(iconVerts.size() / 4));
    if (hotbarIdx < 8)
    {
        float sx0 = barX0 + (hotbarIdx * (slot + gap));
        float sx1 = sx0 + slot;
        float sy0 = barY0;
        float sy1 = sy0 + slot;
        auto [nx0, ny0] = toNDC(sx0, sy0);
        auto [nx1, ny1] = toNDC(sx1, sy1);
        float au0;
        float av0;
        float au1;
        float av1;
        TextureAtlas::uvRect(4, au0, av0, au1, av1);
        atlas.bind(0);
        glUniform4f(glGetUniformLocation(hudShader.id, "uTint"), 1.f, 1.f, 1.f, 0.35f);
        float sel[6 * 4];
        float *p = sel;
        auto quad = [&](float x0, float y0, float x1, float y1)
        {
            *p++ = x0;
            *p++ = y0;
            *p++ = au0;
            *p++ = av0;
            *p++ = x1;
            *p++ = y0;
            *p++ = au1;
            *p++ = av0;
            *p++ = x1;
            *p++ = y1;
            *p++ = au1;
            *p++ = av1;
            *p++ = x0;
            *p++ = y0;
            *p++ = au0;
            *p++ = av0;
            *p++ = x1;
            *p++ = y1;
            *p++ = au1;
            *p++ = av1;
            *p++ = x0;
            *p++ = y1;
            *p++ = au0;
            *p++ = av1;
        };
        quad(nx0, ny0, nx1, ny1);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sel), sel);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

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
    glBufferData(GL_ARRAY_BUFFER, 512 * 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void const *>(2 * sizeof(float)));
    glBindVertexArray(0);
}

void Renderer::drawText(const char *text, float px, float py, int scale, int winW, int winH) const
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
        Font::uvForChar(*p, u0, v0, u1, v1);
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
    std::snprintf(buf, sizeof(buf), "FPS: %d  CHUNKS: %d  View: %d", fps, chunkUpdates, fogLevel);
    float statsY = 4.f + (Font::CHAR_H * verScale) + 4.f;
    drawText(buf, 4.f, statsY, 1, winW, winH);
    constexpr int modeScale = 2;
    const char *modeLabel = placeMode ? "PLACE" : "DIG";
    float modeW = std::strlen(modeLabel) * Font::CHAR_W * modeScale;
    drawText(modeLabel, (float)winW - modeW - 4.f, 4.f, modeScale, winW, winH);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

static constexpr float fogNear[4] = {8.f, 24.f, 48.f, 96.f};
static constexpr float fogFar[4] = {16.f, 40.f, 64.f, 128.f};

void Renderer::rebuildDirty(const World &world, const glm::vec3 &playerPos)
{
    int currentChunkUpdates = 0;
    float maxDist2 = fogFar[fogLevel] * fogFar[fogLevel];

    struct DirtyEntry
    {
        int cx;
        int cz;
        float dist2;
    };

    std::vector<DirtyEntry> dirty;
    for (int cz = 0; cz < World::CHUNKS_Z; cz++)
    {
        for (int cx = 0; cx < World::CHUNKS_X; cx++)
        {
            auto *chunk = const_cast<Chunk *>(world.getChunk(cx, cz));
            if (!chunk || !chunk->dirty)
            {
                continue;
            }

            float dx = (float)((cx * 16) + 8) - playerPos.x;
            float dz = (float)((cz * 16) + 8) - playerPos.z;
            float dist2 = (dx * dx) + (dz * dz);
            if (dist2 > maxDist2)
            {
                continue;
            }

            dirty.push_back({cx, cz, dist2});
        }
    }

    std::sort(dirty.begin(), dirty.end(),
              [](const DirtyEntry &a, const DirtyEntry &b) { return a.dist2 < b.dist2; });
    for (const auto &e : dirty)
    {
        if (currentChunkUpdates >= maxRebuildsPerFrame)
        {
            break;
        }

        auto *chunk = const_cast<Chunk *>(world.getChunk(e.cx, e.cz));
        int idx = (e.cz * World::CHUNKS_X) + e.cx;
        meshes[idx].build(*chunk, world);
        meshes[idx].upload();
        chunk->dirty = false;
        currentChunkUpdates++;
    }

    lastChunkUpdates += currentChunkUpdates;
}

void Renderer::renderGenerating(int winW, int winH)
{
    glViewport(0, 0, winW, winH);
    glClearColor(0.18f, 0.10f, 0.03f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    txtShader.setVec3("uColor", 1.f, 1.f, 1.f);
    const char *msg = "Generating...";
    int scale = 3;
    float tw = std::strlen(msg) * Font::CHAR_W * scale;
    float th = Font::CHAR_H * scale;
    drawText(msg, ((float)winW * 0.5f) - (tw * 0.5f), ((float)winH * 0.5f) - (th * 0.5f), scale, winW, winH);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Renderer::renderPlayerNames(const std::vector<RemotePlayer> &players, const Camera &cam, int winW, int winH)
{
    float aspect = (winH > 0) ? (float)winW / (float)winH : 1.f;
    glm::mat4 vp = cam.viewProjection(aspect);
    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    constexpr int scale = 2;
    for (const auto &p : players)
    {
        if (p.name[0] == '\0')
        {
            continue;
        }

        glm::vec4 clip = vp * glm::vec4(p.vx, p.vy + 0.70f, p.vz, 1.f);
        if (clip.w <= 0.f)
        {
            continue;
        }

        glm::vec3 ndc = glm::vec3(clip) / clip.w;
        if (ndc.z > 1.f)
        {
            continue;
        }

        float sx = ((ndc.x * 0.5f) + 0.5f) * (float)winW;
        float sy = (1.f - ((ndc.y * 0.5f) + 0.5f)) * (float)winH;
        auto nameW = (float)(std::strlen(p.name) * Font::CHAR_W * scale);
        sx -= nameW * 0.5f;
        txtShader.setVec3("uColor", 0.f, 0.f, 0.f);
        drawText(p.name, sx + 2.f, sy + 2.f, scale, winW, winH);
        txtShader.setVec3("uColor", 1.f, 1.f, 1.f);
        drawText(p.name, sx, sy, scale, winW, winH);
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Renderer::renderChat(const std::vector<std::string> &msgs, bool chatOpen, const std::string &buffer, int winW, int winH)
{
    if (msgs.empty() && !chatOpen)
    {
        return;
    }

    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    constexpr int s = 1;
    constexpr float lineH = (Font::CHAR_H * s) + 2.f;
    constexpr float botY = 20.f;
    constexpr int maxClosed = 5;
    int count = chatOpen ? (int)msgs.size() : std::min((int)msgs.size(), maxClosed);
    int startIdx  = (int)msgs.size() - count;
    float msgBotY = (float)winH - botY - (chatOpen ? lineH : 0.f);
    txtShader.setVec3("uColor", 0.9f, 0.9f, 0.9f);
    for (int i = 0; i < count; i++)
    {
        float py = msgBotY - ((float)(count - 1 - i) * lineH);
        drawText(msgs[startIdx + i].c_str(), 4.f, py, s, winW, winH);
    }

    if (chatOpen)
    {
        std::string inputLine = "> " + buffer + "|";
        txtShader.setVec3("uColor", 1.f, 1.f, 0.3f);
        drawText(inputLine.c_str(), 4.f, (float)winH - botY, s, winW, winH);
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Renderer::renderPlayerList(const std::vector<std::string> &names, int winW, int winH)
{
    if (names.empty())
    {
        return;
    }

    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    constexpr int s = 2;
    constexpr float lineH = (Font::CHAR_H * s) + 4.f;
    char header[32];
    std::snprintf(header, sizeof(header), "Players: %d", (int)names.size());
    auto headerW = (float)(std::strlen(header) * Font::CHAR_W * s);
    float textX = ((float)winW - headerW) * 0.5f;
    float textY = 20.f;
    txtShader.setVec3("uColor", 1.f, 1.f, 0.3f);
    drawText(header, textX, textY, s, winW, winH);
    textY += lineH;
    txtShader.setVec3("uColor", 1.f, 1.f, 1.f);
    for (const auto &n : names)
    {
        auto nw = (float)(n.size() * Font::CHAR_W * s);
        drawText(n.c_str(), ((float)winW - nw) * 0.5f, textY, s, winW, winH);
        textY += lineH;
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Renderer::renderPauseMenu(int winW, int winH)
{
    float overlayVerts[12] = {
            -1.f, -1.f, 1.f, -1.f, 1.f, 1.f,
            -1.f, -1.f, 1.f, 1.f, -1.f, 1.f,
    };
    glBindBuffer(GL_ARRAY_BUFFER, xhVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(overlayVerts), overlayVerts);
    _2dShader.use();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    glBindVertexArray(xhVao);
    _2dShader.use();
    glBindVertexArray(0);
    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    constexpr int s = 3;
    float lineH  = Font::CHAR_H * s + 6.f;
    float startY = (float)winH * 0.35f;
    auto drawLine = [&](const char *text, float y, float r, float g, float b)
    {
        float w = std::strlen(text) * Font::CHAR_W * s;
        txtShader.setVec3("uColor", r, g, b);
        drawText(text, ((float)winW * 0.5f) - (w * 0.5f), y, s, winW, winH);
    };
    drawLine("PAUSED", startY, 1.f, 1.f, 0.3f);
    drawLine("Esc      Resume", startY + (lineH * 2), 1.f, 1.f, 1.f);
    drawLine("N        New World", startY + (lineH * 3), 1.f, 1.f, 1.f);
    drawLine("F1-F5    Save slot", startY + (lineH * 4), 1.f, 1.f, 1.f);
    drawLine("F6-F10   Load slot", startY + (lineH * 5), 1.f, 1.f, 1.f);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Renderer::renderFrame(const World &world, const Camera &cam, int winW, int winH, const Renderer::HighlightBlock &hl, float time, int hotbarSize, int hotbarIdx, int fps, int chunkUpdates, bool placeMode, bool underLava, bool underWater, bool showHotbar)
{
    rebuildDirty(world, cam.position);
    float cFogNear;
    float cFogFar;
    float fogR;
    float fogG;
    float fogB;
    if (underLava)
    {
        cFogNear = 1.f;
        cFogFar = 4.f;
        fogR = 0.9f;
        fogG = 0.08f;
        fogB = 0.05f;
    }
    else if (underWater)
    {
        cFogNear = 4.f;
        cFogFar = 14.f;
        fogR = 0.294f;
        fogG = 0.0f;
        fogB = 0.510f;
    }
    else
    {
        cFogNear = fogNear[fogLevel];
        cFogFar = fogFar[fogLevel];
        fogR = 1.0f;
        fogG = 1.0f;
        fogB = 1.0f;
    }

    glViewport(0, 0, winW, winH);
    glClearColor(fogR, fogG, fogB, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float aspect = (winH > 0) ? (float)winW / (float)winH : 1.f;
    glm::mat4 vp = cam.viewProjection(aspect);
    shader.use();
    shader.setMat4("uMVP", glm::value_ptr(vp));
    shader.setInt("uAtlas", 0);
    shader.setFloat("uFogNear", cFogNear);
    shader.setFloat("uFogFar", cFogFar);
    shader.setVec3("uFogColor", fogR, fogG, fogB);
    shader.setFloat("uTime", time);
    shader.setInt("uUnderwater", underWater ? 1 : 0);
    atlas.bind(0);
    int pCx = (int)(cam.position.x / 16.f);
    int pCz = (int)(cam.position.z / 16.f);
    float maxChunkDist = fogFar[fogLevel] / 16.f;
    auto planes = extractFrustumPlanes(vp);
    for (int cz = 0; cz < 16; cz++)
    {
        for (int cx = 0; cx < 16; cx++)
        {
            auto dx = (float)(cx - pCx);
            auto dz = (float)(cz - pCz);
            if (std::max(std::abs(dx), std::abs(dz)) > maxChunkDist)
            {
                continue;
            }

            if (!chunkInFrustum(cx, cz, planes))
            {
                continue;
            }

            meshes[(cz * 16) + cx].drawOpaque();
        }
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    for (int cz = 0; cz < 16; cz++)
    {
        for (int cx = 0; cx < 16; cx++)
        {
            auto dx = (float)(cx - pCx);
            auto dz = (float)(cz - pCz);
            if (std::max(std::abs(dx), std::abs(dz)) > maxChunkDist)
            {
                continue;
            }

            if (!chunkInFrustum(cx, cz, planes))
            {
                continue;
            }

            meshes[(cz * 16) + cx].drawTransparent();
        }
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    renderClouds(glm::value_ptr(vp), time);
    renderHighlight(hl, glm::value_ptr(vp), time, world);
    renderOutline(hl, glm::value_ptr(vp));
    renderCrosshair(winW, winH);
    if (showHotbar)
    {
        renderHUD(winW, winH, hotbarSize, hotbarIdx);
    }

    renderDebug(winW, winH, fps, chunkUpdates, placeMode);
}

void Renderer::buildFullIconAtlas(const BlockType *hotbar, int hotbarSize)
{
    if (fullIconAtlas)
    {
        glDeleteTextures(1, &fullIconAtlas);
        fullIconAtlas = 0;
    }

    std::vector<int> tops(hotbarSize);
    std::vector<int> sides(hotbarSize);
    for (int i = 0; i < hotbarSize; i++)
    {
        BlockType bt = hotbar[i];
        if (bt == BlockType::Water || bt == BlockType::Lava)
        {
            tops[i] = -1;
            sides[i] = -1;
        }
        else
        {
            tops[i] = (int)blockDef(bt).texTop;
            sides[i] = (int)blockDef(bt).texSide;
        }
    }

    fullIconAtlas = atlas.buildIconAtlas(tops.data(), sides.data(), hotbarSize);
}

void Renderer::renderInventory(int winW, int winH, BlockType selected, float mouseX, float mouseY, const BlockType *hotbar, int hotbarSize)
{
    if (!fullIconAtlas)
    {
        return;
    }

    int rows = (hotbarSize + invCols - 1) / invCols;
    auto fw = (float)winW;
    auto fh = (float)winH;
    float totalW = (invCols * invSlot) + ((invCols - 1) * invGap);
    float totalH = (rows * invSlot) + ((rows - 1) * invGap);
    float ox = (fw - totalW) * 0.5f;
    float oy = (fh - totalH) * 0.5f;
    auto ndcX = [&](float sx) { return (sx / fw * 2.f) - 1.f; };
    auto ndcY = [&](float sy) { return 1.f - (sy / fh * 2.f); };
    auto pushQuad = [](std::vector<float> &v,
                       float x0, float y0, float x1, float y1,
                       float u0, float vv0, float u1, float vv1)
    {
        v.insert(v.end(), {
                                  x0, y0, u0, vv1, x1, y0, u1, vv1, x1, y1, u1, vv0,
                                  x0, y0, u0, vv1, x1, y1, u1, vv0, x0, y1, u0, vv0,
                          });
    };

    glBindVertexArray(hudVao);
    glBindBuffer(GL_ARRAY_BUFFER, hudVbo);
    hudShader.use();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float au0;
    float av0;
    float au1;
    float av1;
    TextureAtlas::uvRect(4, au0, av0, au1, av1);
    atlas.bind(0);
    glUniform4f(glGetUniformLocation(hudShader.id, "uTint"), 0.f, 0.f, 0.f, 0.65f);
    float ov[6 * 4] = {
            -1.f, -1.f, au0, av0, 1.f, -1.f, au1, av0, 1.f, 1.f, au1, av1,
            -1.f, -1.f, au0, av0, 1.f,  1.f, au1, av1, -1.f, 1.f, au0, av1,
    };
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ov), ov);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    std::vector<float> bgVerts;
    std::vector<float> iconVerts;
    std::vector<float> hlVerts;
    for (int i = 0; i < hotbarSize; i++)
    {
        int col = i % invCols;
        int row = i / invCols;
        float sx0 = ox + (col * (invSlot + invGap));
        float sy0 = oy + (row * (invSlot + invGap));
        float sx1 = sx0 + invSlot;
        float sy1 = sy0 + invSlot;
        float nx0 = ndcX(sx0);
        float nx1 = ndcX(sx1);
        float ny0 = ndcY(sy1);
        float ny1 = ndcY(sy0);
        pushQuad(bgVerts, nx0, ny0, nx1, ny1, au0, av0, au1, av1);
        float iu0 = (float)i / (float)hotbarSize;
        float iu1 = (float)(i + 1) / (float)hotbarSize;
        pushQuad(iconVerts, nx0, ny0, nx1, ny1, iu0, 0.f, iu1, 1.f);
        bool hovered = mouseX >= sx0 && mouseX < sx1 && mouseY >= sy0 && mouseY < sy1;
        bool isSelected = (hotbar[i] == selected);
        if (hovered || isSelected)
        {
            pushQuad(hlVerts, nx0, ny0, nx1, ny1, au0, av0, au1, av1);
        }
    }

    atlas.bind(0);
    glUniform4f(glGetUniformLocation(hudShader.id, "uTint"), 0.f, 0.f, 0.f, 0.5f);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(bgVerts.size() * sizeof(float)), bgVerts.data());
    glDrawArrays(GL_TRIANGLES, 0, (int)(bgVerts.size() / 4));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fullIconAtlas);
    glUniform4f(glGetUniformLocation(hudShader.id, "uTint"), 1.f, 1.f, 1.f, 1.f);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(iconVerts.size() * sizeof(float)), iconVerts.data());
    glDrawArrays(GL_TRIANGLES, 0, (int)(iconVerts.size() / 4));
    if (!hlVerts.empty())
    {
        atlas.bind(0);
        glUniform4f(glGetUniformLocation(hudShader.id, "uTint"), 1.f, 1.f, 1.f, 0.30f);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(hlVerts.size() * sizeof(float)), hlVerts.data());
        glDrawArrays(GL_TRIANGLES, 0, (int)(hlVerts.size() / 4));
    }

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}