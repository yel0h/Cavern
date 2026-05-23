#include "WorldGen.hpp"
#include "World.hpp"
#include <algorithm>
#include <cmath>

namespace WorldGen
{
    static float hashNoise(int ix, int iz, unsigned int seed)
    {
        unsigned int h = (unsigned int)((ix * 2654435761u) + (iz * 805459861u)) ^ seed;
        h ^= h >> 16;
        h *= 0x45d9f3bu;
        h ^= h >> 16;
        return (float)(h & 0xFFFF) / 65535.f;
    }

    static float hashNoise3(int ix, int iy, int iz, unsigned int seed)
    {
        unsigned int h = (unsigned int)((ix * 2654435761u) + (iy * 3674653429u) + (iz * 805459861u)) ^ seed;
        h ^= h >> 16;
        h *= 0x45d9f3bu;
        h ^= h >> 16;
        return (float) (h & 0xFFFF) / 65535.f;
    }

    static float smoothNoise(float fx, float fz, unsigned int seed)
    {
        int x0 = (int)std::floor(fx);
        int z0 = (int)std::floor(fz);
        float tx = fx - (float)x0;
        float tz = fz - (float)z0;
        float ux = tx * tx * (3.f - (2.f * tx));
        float uz = tz * tz * (3.f - (2.f * tz));
        float v00 = hashNoise(x0, z0, seed);
        float v10 = hashNoise(x0 + 1, z0, seed);
        float v01 = hashNoise(x0, z0 + 1, seed);
        float v11 = hashNoise(x0 + 1, z0 + 1, seed);
        return v00 + (ux * (v10 - v00)) + (uz * (v01 - v00)) + (ux * uz * (v00 - v10 - v01 + v11));
    }

    static float smoothNoise3(float fx, float fy, float fz, unsigned int seed)
    {
        int x0 = (int)std::floor(fx);
        int y0 = (int)std::floor(fy);
        int z0 = (int)std::floor(fz);
        float tx = fx - x0;
        float ty = fy - y0;
        float tz = fz - z0;
        float ux = tx * tx * (3.f - (2.f * tx));
        float uy = ty * ty * (3.f - (2.f * ty));
        float uz = tz * tz * (3.f - (2.f * tz));
        float v000 = hashNoise3(x0, y0, z0, seed);
        float v100 = hashNoise3(x0 + 1, y0, z0, seed);
        float v010 = hashNoise3(x0, y0 + 1, z0, seed);
        float v110 = hashNoise3(x0 + 1, y0 + 1, z0, seed);
        float v001 = hashNoise3(x0, y0, z0 + 1, seed);
        float v101 = hashNoise3(x0 + 1, y0, z0 + 1, seed);
        float v011 = hashNoise3(x0, y0 + 1, z0 + 1, seed);
        float v111 = hashNoise3(x0 + 1, y0 + 1, z0 + 1, seed);
        return (v000 * (1 - ux) * (1 - uy) * (1 - uz)) + (v100 * ux * (1 - uy) * (1 - uz))
               + (v010 * (1 - ux) * uy * (1 - uz)) + (v110 * ux * uy * (1 - uz))
               + (v001 * (1 - ux) * (1 - uy) * uz) + (v101 * ux * (1 - uy) * uz)
               + (v011 * (1 - ux) * uy * uz) + (v111 * ux * uy * uz);
    }

    static float fbm(float wx, float wz, unsigned int seed)
    {
        float scale = 1.f / 32.f;
        float value = 0.f;
        float amp = 0.5f;
        float freq = 1.f;
        for (int oct = 0; oct < 3; oct++)
        {
            value += (smoothNoise(wx * scale * freq, wz * scale * freq, seed + (unsigned int)(oct * 9137)) - 0.5f) * amp;
            amp *= 0.5f;
            freq *= 2.f;
        }

        return value;
    }

    static float fbm3(float wx, float wy, float wz, unsigned int seed)
    {
        float scale = 1.f / 48.f;
        float value = 0.f;
        float amp = 0.5f;
        float freq = 1.f;
        for (int oct = 0; oct < 3; oct++)
        {
            value += (smoothNoise3(wx * scale * freq, wy * scale * freq, wz * scale * freq,
                                   seed + (unsigned int)(oct * 7919)) - 0.5f) * amp;
            amp *= 0.5f;
            freq *= 2.f;
        }

        return value;
    }

    static int computeSurfaceY(float wx, float wz, unsigned int seed)
    {
        float n = fbm(wx, wz, seed);
        return std::clamp(36 + (int)((n + 0.5f) * 22.f), 36, 58);
    }

    void generate(World &world, unsigned int seed)
    {
        for (int wx = 0; wx < World::BLOCK_W; wx++)
        {
            for (int wz = 0; wz < World::BLOCK_D; wz++)
            {
                int surfaceY = computeSurfaceY((float)wx, (float)wz, seed);
                auto soilH = (unsigned int)(wx * 73856093 ^ wz * 19349663 ^ seed);
                int soilDepth  = 1 + (int)(soilH % 3);
                for (int wy = 0; wy < World::BLOCK_H; wy++)
                {
                    BlockType t;
                    if (wy > surfaceY)
                    {
                        t = BlockType::Air;
                    }
                    else if (wy == surfaceY)
                    {
                        t = BlockType::Turf;
                    }
                    else if (wy >= surfaceY - soilDepth)
                    {
                        t = BlockType::Soil;
                    }
                    else
                    {
                        t = BlockType::Stone;
                    }

                    world.setBlock(wx, wy, wz, t);
                }
            }
        }

        for (int wx = 0; wx < World::BLOCK_W; wx++)
        {
            for (int wz = 0; wz < World::BLOCK_D; wz++)
            {
                int sy = computeSurfaceY((float)wx, (float)wz, seed);
                for (int wy = 1; wy <= sy - 4; wy++)
                {
                    float depth = (float)(sy - wy);
                    float bias = std::clamp(depth / 32.f, 0.f, 1.f);
                    float threshold = 0.18f - (bias * 0.10f);
                    float n = fbm3((float)wx, (float)wy, (float)wz, seed + 0x9E3779B9u);
                    if (std::abs(n) < threshold)
                    {
                        world.setBlock(wx, wy, wz, BlockType::Air);
                    }
                }
            }
        }
    }
}