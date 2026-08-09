#include "Renderer.hpp"
#include "../world/World.hpp"
#include "src/entity/SignManager.hpp"
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
    unsigned int h = (unsigned)(gx * 3741225937u) ^ (unsigned)(gz * 805459861u) ^ 0xC10CD5u;
    h ^= h >> 16;
    h *= 0x6b2e47bbu;
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

void Renderer::initOutline()
{
    olShader.build(boxVertSrc, olFragSrc);
    glGenVertexArrays(1, &olVao);
    glGenBuffers(1, &olVbo);
    glBindVertexArray(olVao);
    glBindBuffer(GL_ARRAY_BUFFER, olVbo);
    glBufferData(GL_ARRAY_BUFFER, 24 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void Renderer::renderOutline(const Renderer::HighlightBlock &hl, const float *vp, const World &world)
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
    hudTintLoc = glGetUniformLocation(hudShader.id, "uTint");
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
    glUniform4f(hudTintLoc, 0.f, 0.f, 0.f, 0.55f);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(bgVerts.size() * sizeof(float)), bgVerts.data());
    glDrawArrays(GL_TRIANGLES, 0, (int)(bgVerts.size() / 4));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, iconAtlas);
    glUniform4f(hudTintLoc, 1.f, 1.f, 1.f, 1.f);
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
        glUniform4f(hudTintLoc, 1.f, 1.f, 1.f, 0.35f);
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

void Renderer::renderVitality(int vitality, int maxVitality, int winW, int winH)
{
    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "VIT %d/%d", vitality, maxVitality);
    bool low = vitality <= maxVitality / 4;
    if (low)
    {
        txtShader.setVec3("uColor", 0.85f, 0.2f, 0.15f);
    }
    else
    {
        txtShader.setVec3("uColor", 1.f, 1.f, 1.f);
    }

    constexpr int scale = 2;
    float w = (float)std::strlen(buf) * Font::CHAR_W * scale;
    drawText(buf, (float)winW - w - 4.f, (float)winH - (Font::CHAR_H * scale) - 4.f, scale, winW, winH);
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

void Renderer::renderSigns(const std::vector<Sign> &signs, const Camera &cam, int winW, int winH)
{
    if (signs.empty())
    {
        return;
    }

    float aspect = (winH > 0) ? (float) winW / (float) winH : 1.f;
    glm::mat4 vp = cam.viewProjection(aspect);
    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    constexpr int scale = 2;
    constexpr float maxDist = 20.f;
    const char *lines[3] = {SignManager::line1, SignManager::line2, SignManager::line3};
    for (const auto &s : signs)
    {
        float dist = glm::length(s.position - cam.position);
        if (dist > maxDist)
        {
            continue;
        }

        glm::vec4 clip = vp * glm::vec4(s.position.x, s.position.y + 1.1f, s.position.z, 1.f);
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
        for (int i = 0; i < 3; i++)
        {
            float lineW = (float)(std::strlen(lines[i]) * Font::CHAR_W * scale);
            float lx = sx - (lineW * 0.5f);
            float ly = sy + ((float)i * ((Font::CHAR_H * scale) + 2.f));
            txtShader.setVec3("uColor", 0.f, 0.f, 0.f);
            drawText(lines[i], lx + 2.f, ly + 2.f, scale, winW, winH);
            txtShader.setVec3("uColor", 0.95f, 0.85f, 0.55f);
            drawText(lines[i], lx, ly, scale, winW, winH);
        }
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

void Renderer::renderPlayerList(const std::vector<std::string> &names, int winW, int winH, float mouseX, float mouseY, bool clickable)
{
    if (names.empty())
    {
        return;
    }

    constexpr int s = 2;
    constexpr float lineH = (Font::CHAR_H * s) + 4.f;
    char header[32];
    std::snprintf(header, sizeof(header), "Players: %d", (int)names.size());
    auto headerW = (float)(std::strlen(header) * Font::CHAR_W * s);
    float textX = ((float)winW - headerW) * 0.5f;
    float textY = 20.f;
    if (clickable)
    {
        float rowY = textY + lineH;
        for (const auto &n : names)
        {
            auto nw = (float)(n.size() * Font::CHAR_W * s);
            float rx0 = (((float)winW - nw) * 0.5f) - 4.f;
            float rx1 = rx0 + nw + 8.f;
            if (mouseX >= rx0 && mouseX < rx1 && mouseY >= rowY && mouseY < rowY + lineH)
            {
                _2dShader.use();
                glBindVertexArray(xhVao);
                glBindBuffer(GL_ARRAY_BUFFER, xhVbo);
                float nx0 = (rx0 / (float)winW * 2.f) - 1.f;
                float nx1 = (rx1 / (float)winW * 2.f) - 1.f;
                float ny0 = 1.f - ((rowY + lineH) / (float)winH * 2.f);
                float ny1 = 1.f - (rowY / (float)winH * 2.f);
                float hv[12] = {nx0,ny0, nx1,ny0, nx1,ny1, nx0,ny0, nx1,ny1, nx0,ny1};
                glDisable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(hv), hv);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
                break;
            }

            rowY += lineH;
        }
    }

    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

void Renderer::drawButton(float x0, float y0, float x1, float y1, bool hovered, bool on, const char *label, int winW, int winH)
{
    auto fw = (float)winW;
    auto fh = (float)winH;
    auto ndcX = [&](float sx) { return (sx / fw * 2.f) - 1.f; };
    auto ndcY = [&](float sy) { return 1.f - (sy / fh * 2.f); };
    auto pushQuad = [](std::vector<float> &v,
                       float qx0, float qy0, float qx1, float qy1,
                       float u0, float vv0, float u1, float vv1)
    {
        v.insert(v.end(), {
                                  qx0, qy0, u0, vv1, qx1, qy0, u1, vv1, qx1, qy1, u1, vv0,
                                  qx0, qy0, u0, vv1, qx1, qy1, u1, vv0, qx0, qy1, u0, vv0,
                          });
    };

    float au0;
    float av0;
    float au1;
    float av1;
    TextureAtlas::uvRect(4, au0, av0, au1, av1);
    glBindVertexArray(hudVao);
    glBindBuffer(GL_ARRAY_BUFFER, hudVbo);
    hudShader.use();
    atlas.bind(0);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    {
        std::vector<float> v;
        pushQuad(v, ndcX(x0 - 2.f), ndcY(y1 + 2.f), ndcX(x1 + 2.f), ndcY(y0 - 2.f), au0, av0, au1, av1);
        glUniform4f(hudTintLoc, 0.08f, 0.09f, 0.12f, 0.92f);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(v.size() * sizeof(float)), v.data());
        glDrawArrays(GL_TRIANGLES, 0, (int)(v.size() / 4));
    }

    {
        std::vector<float> v;
        pushQuad(v, ndcX(x0), ndcY(y1), ndcX(x1), ndcY(y0), au0, av0, au1, av1);
        if (on)
        {
            glUniform4f(hudTintLoc, 0.55f, 0.42f, 0.15f, 0.92f);
        }
        else
        {
            glUniform4f(hudTintLoc, 0.20f, 0.22f, 0.28f, 0.88f);
        }

        glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(v.size() * sizeof(float)), v.data());
        glDrawArrays(GL_TRIANGLES, 0, (int)(v.size() / 4));
    }

    if (hovered)
    {
        std::vector<float> v;
        pushQuad(v, ndcX(x0), ndcY(y1), ndcX(x1), ndcY(y0), au0, av0, au1, av1);
        glUniform4f(hudTintLoc, 0.85f, 0.85f, 0.90f, 0.25f);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(v.size() * sizeof(float)), v.data());
        glDrawArrays(GL_TRIANGLES, 0, (int)(v.size() / 4));
    }

    glBindVertexArray(0);
    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    constexpr int s = 1;
    float labelW = (float)std::strlen(label) * Font::CHAR_W * s;
    float labelH = Font::CHAR_H * s;
    float lx = ((x0 + x1) * 0.5f) - (labelW * 0.5f);
    float ly = ((y0 + y1) * 0.5f) - (labelH * 0.5f);
    if (on)
    {
        txtShader.setVec3("uColor", 0.05f, 0.05f, 0.05f);
    }
    else
    {
        txtShader.setVec3("uColor", 1.f, 1.f, 1.f);
    }

    drawText(label, lx, ly, s, winW, winH);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Renderer::renderPauseMenu(int winW, int winH, float mouseX, float mouseY)
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
    constexpr float btnW = 200.f;
    constexpr float btnH = 32.f;
    float bx0 = ((float)winW * 0.5f) - (btnW * 0.5f);
    float bx1 = bx0 + btnW;
    float by0 = startY + (lineH * 6.5f);
    float by1 = by0 + btnH;
    bool hovered = mouseX >= bx0 && mouseX < bx1 && mouseY >= by0 && mouseY < by1;
    drawButton(bx0, by0, bx1, by1, hovered, false, "OPTIONS", winW, winH);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Renderer::renderDeathScreen(int winW, int winH, float mouseX, float mouseY, int score)
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    constexpr int s = 4;
    float lineH = (Font::CHAR_H * s) + 6.f;
    float startY = (float)winH * 0.3f;
    auto drawLine = [&](const char *text, float y, int scale, float r, float g, float b)
    {
        float w = std::strlen(text) * Font::CHAR_W * scale;
        txtShader.setVec3("uColor", r, g, b);
        drawText(text, ((float)winW * 0.5f) - (w * 0.5f), y, scale, winW, winH);
    };
    drawLine("BURIED", startY, s, 0.85f, 0.2f, 0.15f);
    char scoreBuf[32];
    std::snprintf(scoreBuf, sizeof(scoreBuf), "SCORE: %d", score);
    drawLine(scoreBuf, startY + (lineH * 1.6f), 2, 1.f, 1.f, 1.f);
    constexpr float btnW = 260.f;
    constexpr float btnH = 32.f;
    float bx0 = ((float)winW * 0.5f) - (btnW * 0.5f);
    float bx1 = bx0 + btnW;
    float by0 = startY + (lineH * 3.2f);
    float by1 = by0 + btnH;
    bool hovered = mouseX >= bx0 && mouseX < bx1 && mouseY >= by0 && mouseY < by1;
    drawButton(bx0, by0, bx1, by1, hovered, false, "START A NEW DESCENT", winW, winH);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

static std::string keyName(int key)
{
    switch (key)
    {
        case GLFW_KEY_SPACE:
            return "SPACE";

        case GLFW_KEY_ENTER:
            return "ENTER";

        case GLFW_KEY_TAB:
            return "TAB";

        case GLFW_KEY_ESCAPE:
            return "ESCAPE";

        case GLFW_KEY_BACKSPACE:
            return "BACKSPACE";

        case GLFW_KEY_LEFT_SHIFT:
            return "L-SHIFT";

        case GLFW_KEY_RIGHT_SHIFT:
            return "R-SHIFT";

        case GLFW_KEY_LEFT_CONTROL:
            return "L-CTRL";

        case GLFW_KEY_RIGHT_CONTROL:
            return "R-CTRL";

        case GLFW_KEY_LEFT_ALT:
            return "L-ALT";

        case GLFW_KEY_RIGHT_ALT:
            return "R-ALT";

        case GLFW_KEY_UP:
            return "UP";

        case GLFW_KEY_DOWN:
            return "DOWN";

        case GLFW_KEY_LEFT:
            return "LEFT";

        case GLFW_KEY_RIGHT:
            return "RIGHT";

        case GLFW_KEY_F1:
            return "F1";

        case GLFW_KEY_F2:
            return "F2";

        case GLFW_KEY_F3:
            return "F3";

        case GLFW_KEY_F4:
            return "F4";

        case GLFW_KEY_F5:
            return "F5";

        case GLFW_KEY_F6:
            return "F6";

        case GLFW_KEY_F7:
            return "F7";

        case GLFW_KEY_F8:
            return "F8";

        case GLFW_KEY_F9:
            return "F9";

        case GLFW_KEY_F10:
            return "F10";

        case GLFW_KEY_F11:
            return "F11";

        case GLFW_KEY_F12:
            return "F12";

        default:
            break;
    }

    const char *nm = glfwGetKeyName(key, 0);
    if (nm && nm[0])
    {
        std::string s(nm);
        for (auto &c : s)
        {
            c = (char)std::toupper((unsigned char)c);
        }

        return s;
    }

    return "?";
}

static const char *fogLevelNames[4] = {"TINY", "SHORT", "NORMAL", "FAR"};

void Renderer::renderOptionsMenu(int winW, int winH, float mouseX, float mouseY, int pendingRow, const Settings &s)
{
    constexpr float startY = 30.f;
    constexpr float rowH = 24.f;
    constexpr float rowGap = 6.f;
    constexpr float step = rowH + rowGap;
    constexpr float colW = 300.f;
    constexpr float colGap = 20.f;
    float colLeftX = ((float)winW * 0.5f) - (colGap * 0.5f) - colW;
    float colRightX = ((float)winW * 0.5f) + (colGap * 0.5f);
    txtShader.use();
    txtShader.setInt("uFont", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font.texId);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    constexpr int titleS = 2;
    const char *title = "OPTIONS";
    float titleW = (float)std::strlen(title) * Font::CHAR_W * titleS;
    txtShader.setVec3("uColor", 1.f, 1.f, 0.3f);
    drawText(title, ((float)winW * 0.5f) - (titleW * 0.5f), 4.f, titleS, winW, winH);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    struct Row
    {
        const char *label;
        std::string value;
    };

    Row leftRows[13] = {
            {"Forward", keyName(s.keyForward)},
            {"Backward", keyName(s.keyBackward)},
            {"Left", keyName(s.keyLeft)},
            {"Right", keyName(s.keyRight)},
            {"Jump", keyName(s.keyJump)},
            {"Set Spawn", keyName(s.keySave)},
            {"Fog Cycle", keyName(s.keyCycleFog)},
            {"New World", keyName(s.keyNewLevel)},
            {"Fullscreen", keyName(s.keyFullscreen)},
            {"Chat", keyName(s.keyChat)},
            {"Inventory", keyName(s.keyInventory)},
            {"Throw Bolt", keyName(s.keyThrowBolt)},
            {"Place Sign", keyName(s.keyPlaceSign)},
    };
    for (int i = 0; i < 13; i++)
    {
        float y0 = startY + (i * step);
        float y1 = y0 + rowH;
        float x0 = colLeftX;
        float x1 = colLeftX + colW;
        bool hovered = mouseX >= x0 && mouseX < x1 && mouseY >= y0 && mouseY < y1;
        char buf[48];
        if (pendingRow == i)
        {
            std::snprintf(buf, sizeof(buf), "%s: Press a key...", leftRows[i].label);
        }
        else
        {
            std::snprintf(buf, sizeof(buf), "%s: %s", leftRows[i].label, leftRows[i].value.c_str());
        }

        drawButton(x0, y0, x1, y1, hovered, pendingRow == i, buf, winW, winH);
    }

    struct ToggleRow
    {
        const char *label;
        bool on;
        std::string value;
    };

    char distBuf[16];
    std::snprintf(distBuf, sizeof(distBuf), "%s", fogLevelNames[s.renderDistance % 4]);
    ToggleRow rightRows[7] = {
            {"Render Distance", false, distBuf},
            {"Invert Mouse", s.invertMouse, s.invertMouse ? "ON" : "OFF"},
            {"Sound Effects", s.soundEnabled, s.soundEnabled ? "ON" : "OFF"},
            {"Music", s.musicEnabled, s.musicEnabled ? "ON" : "OFF"},
            {"Show FPS", s.showFps, s.showFps ? "ON" : "OFF"},
            {"View Bobbing", s.viewBobbing, s.viewBobbing ? "ON" : "OFF"},
            {"Back", false, ""},
    };
    for (int j = 0; j < 7; j++)
    {
        float y0 = startY + (j * step);
        float y1 = y0 + rowH;
        float x0 = colRightX;
        float x1 = colRightX + colW;
        bool hovered = mouseX >= x0 && mouseX < x1 && mouseY >= y0 && mouseY < y1;
        char buf[48];
        if (rightRows[j].value.empty())
        {
            std::snprintf(buf, sizeof(buf), "%s", rightRows[j].label);
        }
        else
        {
            std::snprintf(buf, sizeof(buf), "%s: %s", rightRows[j].label, rightRows[j].value.c_str());
        }

        drawButton(x0, y0, x1, y1, hovered, rightRows[j].on, buf, winW, winH);
    }
}

void Renderer::renderFrame(const World &world, const Camera &cam, int winW, int winH, const Renderer::HighlightBlock &hl, float time, int hotbarSize, int hotbarIdx, int fps, int chunkUpdates, bool placeMode, bool underLava, bool underWater, bool showHotbar, int vitality, int maxVitality)
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
        fogR = 0.78f;
        fogG = 0.80f;
        fogB = 0.82f;
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
    renderOutline(hl, glm::value_ptr(vp), world);
    renderCrosshair(winW, winH);
    if (showHotbar)
    {
        renderHUD(winW, winH, hotbarSize, hotbarIdx);
    }

    renderVitality(vitality, maxVitality, winW, winH);
    if (showFps)
    {
        renderDebug(winW, winH, fps, chunkUpdates, placeMode);
    }
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
    glUniform4f(hudTintLoc, 0.5f, 0.5f, 0.5f, 0.65f);
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
    glUniform4f(hudTintLoc, 0.f, 0.f, 0.f, 0.5f);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(bgVerts.size() * sizeof(float)), bgVerts.data());
    glDrawArrays(GL_TRIANGLES, 0, (int)(bgVerts.size() / 4));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fullIconAtlas);
    glUniform4f(hudTintLoc, 1.f, 1.f, 1.f, 1.f);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(iconVerts.size() * sizeof(float)), iconVerts.data());
    glDrawArrays(GL_TRIANGLES, 0, (int)(iconVerts.size() / 4));
    if (!hlVerts.empty())
    {
        atlas.bind(0);
        glUniform4f(hudTintLoc, 1.f, 1.f, 1.f, 0.30f);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(hlVerts.size() * sizeof(float)), hlVerts.data());
        glDrawArrays(GL_TRIANGLES, 0, (int)(hlVerts.size() / 4));
    }

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}