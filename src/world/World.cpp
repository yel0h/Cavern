#include "World.hpp"
#include "Lighting.hpp"
#include "WorldGen.hpp"
#include <fstream>

static constexpr unsigned int magic = 0x4341564E;
static constexpr unsigned int version = 1;

World::World()
{
    for (int cz = 0; cz < CHUNKS_Z; cz++)
    {
        for (int cx = 0; cx < CHUNKS_X; cx++)
        {
            auto &c = chunks[chunkIdx(cx, cz)];
            c = std::make_unique<Chunk>();
            c->x = cx;
            c->z = cz;
        }
    }
}

Chunk *World::getChunk(int cx, int cz)
{
    if (cx < 0 || cx >= CHUNKS_X || cz < 0 || cz >= CHUNKS_Z)
    {
        return nullptr;
    }

    return chunks[chunkIdx(cx, cz)].get();
}

const Chunk *World::getChunk(int cx, int cz) const
{
    if (cx < 0 || cx >= CHUNKS_X || cz < 0 || cz >= CHUNKS_Z)
    {
        return nullptr;
    }

    return chunks[chunkIdx(cx, cz)].get();
}

bool World::inBounds(int wx, int wy, int wz)
{
    return wx >= 0 && wx < BLOCK_W && wy >= 0 && wy < BLOCK_H && wz >= 0 && wz < BLOCK_D;
}

BlockType World::getBlock(int wx, int wy, int wz) const
{
    if (!inBounds(wx, wy, wz))
    {
        return BlockType::Air;
    }

    const Chunk *c = getChunk(wx / Chunk::WIDTH, wz / Chunk::DEPTH);
    if (!c)
    {
        return BlockType::Air;
    }

    return c->get(wx % Chunk::WIDTH, wy, wz % Chunk::DEPTH);
}

void World::setBlock(int wx, int wy, int wz, BlockType t)
{
    if (!inBounds(wx, wy, wz))
    {
        return;
    }

    int cx = wx / Chunk::WIDTH;
    int cz = wz / Chunk::DEPTH;
    int lx = wx % Chunk::WIDTH;
    int lz = wz % Chunk::DEPTH;
    Chunk* c = getChunk(cx, cz);
    if (!c)
    {
        return;
    }

    c->set(lx, wy, lz, t);
    c->dirty = true;
    if (lx == 0)
    {
        if (auto *n = getChunk(cx - 1, cz))
        {
            n->dirty = true;
        }
    }

    if (lx == Chunk::WIDTH - 1)
    {
        if (auto *n = getChunk(cx + 1, cz))
        {
            n->dirty = true;
        }
    }

    if (lz == 0)
    {
        if (auto *n = getChunk(cx, cz - 1))
        {
            n->dirty = true;
        }
    }

    if (lz == Chunk::DEPTH - 1)
    {
        if (auto *n = getChunk(cx, cz + 1))
        {
            n->dirty = true;
        }
    }
}

unsigned char World::getLight(int wx, int wy, int wz) const
{
    if (!inBounds(wx, wy, wz))
    {
        return 1;
    }

    const Chunk *c = getChunk(wx / Chunk::WIDTH, wz / Chunk::DEPTH);
    if (!c)
    {
        return 1;
    }

    return c->getLight(wx % Chunk::WIDTH, wy, wz % Chunk::DEPTH);
}

void World::setLight(int wx, int wy, int wz, unsigned char v)
{
    if (!inBounds(wx, wy, wz))
    {
        return;
    }

    Chunk *c = getChunk(wx / Chunk::WIDTH, wz / Chunk::DEPTH);
    if (!c)
    {
        return;
    }

    c->setLight(wx % Chunk::WIDTH, wy, wz % Chunk::DEPTH, v);
}

void World::generate(unsigned int seed)
{
    WorldGen::generate(*this, seed);
    Lighting::propagate(*this);
    for (auto &c : chunks)
    {
        if (c)
        {
            c->dirty = true;
        }
    }
}

bool World::save(const char *path) const
{
    std::ofstream f(path, std::ios::binary);
    if (!f)
    {
        return false;
    }

    f.write(reinterpret_cast<char const *>(&magic), sizeof(magic));
    f.write(reinterpret_cast<char const *>(&version), sizeof(version));
    for (auto &c : chunks)
    {
        if (!c)
        {
            continue;
        }

        f.write(reinterpret_cast<char const *>(c->blocks.data()), c->blocks.size());
    }

    return true;
}

bool World::load(const char *path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        return false;
    }

    unsigned int fMagic = 0;
    unsigned int fVersion = 0;
    f.read(reinterpret_cast<char *>(&fMagic), sizeof(fMagic));
    f.read(reinterpret_cast<char *>(&fVersion), sizeof(fVersion));
    if (fMagic != magic || fVersion != version)
    {
        return false;
    }

    for (auto &c : chunks)
    {
        if (!c)
        {
            continue;
        }

        f.read(reinterpret_cast<char *>(c->blocks.data()), c->blocks.size());
    }

    Lighting::propagate(*this);
    for (auto &c : chunks)
    {
        if (c)
        {
            c->dirty = true;
        }
    }

    return true;
}