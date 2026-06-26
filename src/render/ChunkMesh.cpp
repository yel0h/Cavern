#include "ChunkMesh.hpp"
#include "TextureAtlas.hpp"
#include "../world/Chunk.hpp"
#include "../world/World.hpp"

static const int faceNormals[6][3] = {
        {0, 1, 0},
        {0, -1, 0},
        {-1, 0, 0},
        {1, 0, 0},
        {0, 0, -1},
        {0, 0, 1},
};

static const int faceCorners[6][4][3] = {
        {{0, 1, 0}, {0, 1, 1}, {1, 1, 1}, {1, 1, 0}},
        {{0, 0, 1}, {0, 0, 0}, {1, 0, 0}, {1, 0, 1}},
        {{0, 0, 1}, {0, 1, 1}, {0, 1, 0}, {0, 0, 0}},
        {{1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 1}},
        {{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}},
        {{1, 0, 1}, {1, 1, 1}, {0, 1, 1}, {0, 0, 1}},
};

static const float faceUVs[4][2] = {{0.f, 1.f}, {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f},};

void ChunkMesh::addFace(float x, float y, float z, int face, float u0, float v0, float u1, float v1, float light, float flags)
{
    auto base = (unsigned int)verts.size();
    for (int i = 0; i < 4; i++)
    {
        Vertex vtx{};
        vtx.x = x + faceCorners[face][i][0];
        vtx.y = y + faceCorners[face][i][1];
        vtx.z = z + faceCorners[face][i][2];
        vtx.u = u0 + ((u1 - u0) * faceUVs[i][0]);
        vtx.v = v0 + ((v1 - v0) * faceUVs[i][1]);
        vtx.light = light;
        vtx.flags = flags;
        verts.push_back(vtx);
    }

    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
}

void ChunkMesh::build(const Chunk &chunk, const World &world)
{
    verts.clear();
    indices.clear();
    int ox = chunk.x * Chunk::WIDTH;
    int oz = chunk.z * Chunk::DEPTH;
    for (int y = 0; y < Chunk::HEIGHT; y++)
    {
        for (int z = 0; z < Chunk::DEPTH; z++)
        {
            for (int x = 0; x < Chunk::WIDTH; x++)
            {
                BlockType bt = chunk.get(x, y, z);
                if (bt == BlockType::Air)
                {
                    continue;
                }

                const BlockDef &def = blockDef(bt);
                int wx = ox + x;
                int wy = y;
                int wz = oz + z;
                auto light = (float)world.getLight(wx, wy, wz);
                if (blockDef(bt).transparent && !blockDef(bt).liquid)
                {
                    light = 1.0f;
                }

                for (int f = 0; f < 6; f++)
                {
                    int nx = wx + faceNormals[f][0];
                    int ny = wy + faceNormals[f][1];
                    int nz = wz + faceNormals[f][2];
                    BlockType nb = world.getBlock(nx, ny, nz);
                    bool sameFluid = blockDef(bt).liquid && nb == bt;
                    bool liquidFace = blockDef(bt).liquid && blockDef(nb).liquid;
                    if (blockDef(nb).opaque || sameFluid || liquidFace)
                    {
                        continue;
                    }

                    unsigned char tileIdx;
                    if (f == 0)
                    {
                        tileIdx = def.texTop;
                    }
                    else if (f == 1)
                    {
                        tileIdx = def.texBottom;
                    }
                    else
                    {
                        tileIdx = def.texSide;
                    }

                    float u0, v0, u1, v1;
                    TextureAtlas::uvRect(tileIdx, u0, v0, u1, v1);
                    float flags = 0.f;
                    if (blockDef(bt).liquid)
                    {
                        flags = 1.f;
                    }
                    else if (bt == BlockType::Glaze)
                    {
                        flags = 2.f;
                    }

                    addFace((float)wx, (float)wy, (float)wz, f, u0, v0, u1, v1, light, flags);
                }
            }
        }
    }

    indexCount = (int)indices.size();
}

void ChunkMesh::upload()
{
    if (indexCount == 0)
    {
        return;
    }

    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, light));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, flags));
    glBindVertexArray(0);
    verts.clear();
    indices.clear();
}

void ChunkMesh::draw() const
{
    if (vao == 0 || indexCount == 0)
    {
        return;
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void ChunkMesh::free()
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

    if (ibo)
    {
        glDeleteBuffers(1, &ibo);
        ibo = 0;
    }

    indexCount = 0;
}