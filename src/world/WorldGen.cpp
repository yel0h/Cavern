#include "WorldGen.hpp"
#include "World.hpp"
#include <algorithm>
#include <cmath>

namespace WorldGen
{
    static float hashNoise(int ix, int iz, unsigned int seed)
    {
        unsigned int h = (unsigned int)((ix * 1619) + (iz * 31337)) ^ seed;
        h ^= h >> 16;
        h *= 0x45d9f3bu;
        h ^= h >> 16;
        return (float)(h & 0xFFFF) / 65535.f;
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

    void generate(World &world, unsigned int seed)
    {
        for (int wx = 0; wx < World::BLOCK_W; wx++)
        {
            for (int wz = 0; wz < World::BLOCK_D; wz++)
            {
                float n = fbm((float)wx, (float)wz, seed);
                int surfaceY = std::clamp(36 + (int)((n + 0.5f) * 22.f), 36, 58);
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
    }
}