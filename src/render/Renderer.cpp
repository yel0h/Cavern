#include "Renderer.hpp"
#include "../world/World.hpp"
#include <glm/gtc/type_ptr.hpp>

void Renderer::init()
{
    shader.build(vertSrc, fragSrc);
    atlas.build();
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

void Renderer::renderFrame(const World &world, const Camera &cam, int winW, int winH)
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
}