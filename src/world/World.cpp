#include "World.hpp"
#include "Lighting.hpp"
#include "WorldGen.hpp"

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

    Chunk *c = getChunk(wx / Chunk::WIDTH, wz / Chunk::DEPTH);
    if (!c)
    {
        return;
    }

    c->set(wx % Chunk::WIDTH, wy, wz % Chunk::DEPTH, t);
    c->dirty = true;
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
    WorldGen::turfPass(*this);
    WorldGen::cavePass(*this, seed);
    Lighting::propagate(*this);
    WorldGen::turfPass(*this);
    for (auto &c : chunks)
    {
        if (c)
        {
            c->dirty = true;
        }
    }
}