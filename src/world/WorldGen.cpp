#include "WorldGen.hpp"
#include "World.hpp"
#include <algorithm>
#include <cmath>

namespace
{
    float hashToFloat(unsigned int h)
    {
        return (h & 0xFFFFFF) / float(0xFFFFFF);
    }

    unsigned int hash2(int x, int z, unsigned int seed)
    {
        unsigned int h = seed;
        h ^= (unsigned int)x * 2654435761u;
        h ^= (unsigned int)z * 805459861u;
        h *= 1234567891u;
        return h;
    }

    unsigned int hash3(int x, int y, int z, unsigned int seed)
    {
        unsigned int h = seed;
        h ^= (unsigned int)x * 2654435761u;
        h ^= (unsigned int)y * 1234567891u;
        h ^= (unsigned int)z * 805459861u;
        h *= 2246822519u;
        return h;
    }

    float smoothstep(float t)
    {
        return t * t * (3.f - (2.f * t));
    }

    float lerp(float a, float b, float t)
    {
        return a + ((b - a) * t);
    }

    float valueNoise2D(float fx, float fz, unsigned int seed)
    {
        int x0 = (int)std::floor(fx);
        int z0 = (int)std::floor(fz);
        float tx = smoothstep(fx - x0);
        float tz = smoothstep(fz - z0);
        float v00 = (hashToFloat(hash2(x0, z0, seed)) * 2.f) - 1.f;
        float v10 = (hashToFloat(hash2(x0 + 1, z0, seed)) * 2.f) - 1.f;
        float v01 = (hashToFloat(hash2(x0, z0 + 1, seed)) * 2.f) - 1.f;
        float v11 = (hashToFloat(hash2(x0 + 1, z0 + 1, seed)) * 2.f) - 1.f;
        return lerp(lerp(v00, v10, tx), lerp(v01, v11, tx), tz);
    }

    float fractalNoise2D(float fx, float fz, unsigned int seed)
    {
        float val = 0.f;
        float amp = 0.5f;
        float freq = 1.f;
        for (int i = 0; i < 4; i++)
        {
            val += amp * valueNoise2D(fx * freq, fz * freq, seed + (i * 137u));
            amp *= 0.5f;
            freq *= 2.f;
        }

        return val;
    }

    float valueNoise3D(float fx, float fy, float fz, unsigned int seed)
    {
        int x0 = (int)std::floor(fx);
        int y0 = (int)std::floor(fy);
        int z0 = (int)std::floor(fz);
        float tx = smoothstep(fx - x0);
        float ty = smoothstep(fy - y0);
        float tz = smoothstep(fz - z0);
        auto h = [&](int xi, int yi, int zi)
        {
            return (hashToFloat(hash3(xi, yi, zi, seed)) * 2.f) - 1.f;
        };

        float v000 = h(x0, y0, z0);
        float v100 = h(x0 + 1, y0, z0);
        float v010 = h(x0, y0 + 1, z0);
        float v110 = h(x0 + 1, y0 + 1, z0);
        float v001 = h(x0, y0, z0 + 1);
        float v101 = h(x0 + 1, y0, z0 + 1);
        float v011 = h(x0, y0 + 1, z0 + 1);
        float v111 = h(x0 + 1, y0 + 1, z0 + 1);
        float x0y0 = lerp(v000, v100, tx);
        float x1y0 = lerp(v010, v110, tx);
        float x0y1 = lerp(v001, v101, tx);
        float x1y1 = lerp(v011, v111, tx);
        return lerp(lerp(x0y0, x1y0, ty), lerp(x0y1, x1y1, ty), tz);
    }

    float fractalNoise3D(float fx, float fy, float fz, unsigned int seed)
    {
        float val = 0.f;
        float amp = 0.5f;
        float freq = 1.f;
        for (int i = 0; i < 2; i++)
        {
            val += amp * valueNoise3D(fx * freq, fy * freq, fz * freq, seed + (i * 257u));
            amp *= 0.5f;
            freq *= 2.f;
        }

        return val;
    }
}

namespace WorldGen
{
    void generate(World &world, unsigned int seed)
    {
        for (int wx = 0; wx < World::BLOCK_W; wx++)
        {
            for (int wz = 0; wz < World::BLOCK_D; wz++)
            {
                float n = fractalNoise2D(wx / 64.f, wz / 64.f, seed);
                int surfaceY = 48 + (int)((n + 1.f) * 0.5f * 15.f);
                surfaceY = std::clamp(surfaceY, 0, World::BLOCK_H - 1);
                for (int wy = 0; wy < World::BLOCK_H; wy++)
                {
                    BlockType t = (wy <= surfaceY) ? BlockType::Stone : BlockType::Air;
                    world.setBlock(wx, wy, wz, t);
                }
            }
        }
    }

    void turfPass(World &world)
    {
        for (int wx = 0; wx < World::BLOCK_W; wx++)
        {
            for (int wz = 0; wz < World::BLOCK_D; wz++)
            {
                for (int wy = World::BLOCK_H - 1; wy >= 0; wy--)
                {
                    if (world.getBlock(wx, wy, wz) == BlockType::Stone)
                    {
                        if (world.getLight(wx, wy, wz) == 1 && wy >= 57)
                        {
                            world.setBlock(wx, wy, wz, BlockType::Turf);
                        }

                        break;
                    }
                }
            }
        }
    }

    void cavePass(World &world, unsigned int seed)
    {
        unsigned int caveSeed = seed + 0xDEADBEEFu;
        for (int wx = 0; wx < World::BLOCK_W; wx++)
        {
            for (int wz = 0; wz < World::BLOCK_D; wz++)
            {
                int surfaceY = 0;
                for (int wy = World::BLOCK_H - 1; wy >= 0; wy--)
                {
                    BlockType b = world.getBlock(wx, wy, wz);
                    if (b == BlockType::Stone || b == BlockType::Turf)
                    {
                        surfaceY = wy;
                        break;
                    }
                }

                for (int wy = 1; wy <= surfaceY; wy++)
                {
                    if (world.getBlock(wx, wy, wz) != BlockType::Stone)
                    {
                        continue;
                    }

                    float n = fractalNoise3D(wx / 16.f, wy / 8.f, wz / 16.f, caveSeed);
                    if (n > 0.3f)
                    {
                        world.setBlock(wx, wy, wz, BlockType::Air);
                    }
                }
            }
        }
    }
}