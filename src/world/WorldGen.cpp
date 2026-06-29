#include "WorldGen.hpp"
#include "World.hpp"
#include <algorithm>
#include <cmath>

namespace WorldGen
{
    static constexpr int oceanLevel = 34;

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

    static float fbm3(float wx, float wy, float wz, unsigned int seed, int octaves = 3)
    {
        float scale = 1.f / 48.f;
        float value = 0.f;
        float amp = 0.5f;
        float freq = 1.f;
        for (int oct = 0; oct < octaves; oct++)
        {
            value += (smoothNoise3(wx * scale * freq, wy * scale * freq, wz * scale * freq,
                                   seed + (unsigned int)(oct * 7919)) - 0.5f) * amp;
            amp *= 0.5f;
            freq *= 2.f;
        }

        return value;
    }

    static float islandFalloff(float wx, float wz)
    {
        float cx = World::BLOCK_W * 0.5f;
        float cz = World::BLOCK_D * 0.5f;
        float dx = (wx - cx) / cx;
        float dz = (wz - cz) / cz;
        float d = std::min(1.f, std::sqrt((dx * dx) + (dz * dz)));
        float f = 1.f - (d * d);
        return std::max(0.f, f);
    }

    static int computeSurfaceY(float wx, float wz, unsigned int seed)
    {
        float n = fbm(wx, wz, seed);
        float falloff = islandFalloff(wx, wz);
        int base = 36 + (int)((n + 0.5f) * 22.f);
        base = std::clamp(base, 36, 58);
        float blended = ((float)(base - oceanLevel) * falloff) + (float)oceanLevel;
        return std::clamp((int)blended, oceanLevel - 3, 58);
    }

    static void placeTree(World &world, int wx, int sy, int wz, int height)
    {
        for (int dy = 1; dy <= height; dy++)
        {
            int wy = sy + dy;
            if (wy >= World::BLOCK_H)
            {
                break;
            }

            world.setBlock(wx, wy, wz, BlockType::Timber);
        }

        int top = sy + height;
        static constexpr int radii[5] = {1, 2, 2, 1, 1};
        for (int i = 0; i < 5; i++)
        {
            int dy = i - 2;
            int cy = top + dy;
            if (cy < 1 || cy >= World::BLOCK_H)
            {
                continue;
            }

            int radius = radii[i];
            for (int dx = -radius; dx <= radius; dx++)
            {
                for (int dz = -radius; dz <= radius; dz++)
                {
                    if (dx == 0 && dz == 0 && dy <= 0)
                    {
                        continue;
                    }

                    if (radius == 2 && std::abs(dx) == 2 && std::abs(dz) == 2)
                    {
                        continue;
                    }

                    int nx = wx + dx;
                    int nz = wz + dz;
                    if (nx < 1 || nx >= World::BLOCK_W - 1 || nz < 1 || nz >= World::BLOCK_D - 1)
                    {
                        continue;
                    }

                    if (world.getBlock(nx, cy, nz) == BlockType::Air)
                    {
                        world.setBlock(nx, cy, nz, BlockType::Sapling);
                    }
                }
            }
        }
    }

    static void placeOreVein(World &world, int cx, int cy, int cz, BlockType ore, int blobSize, unsigned int seed)
    {
        int bx = cx;
        int by = cy;
        int bz = cz;
        static const int ddx[] = {1, -1, 0, 0, 0, 0};
        static const int ddy[] = {0, 0, 1, -1, 0, 0};
        static const int ddz[] = {0, 0, 0, 0, 1, -1};
        unsigned int rng = seed ^ (unsigned int)(cx * 73856093 ^ cy * 2097143 ^ cz * 19349663);
        for (int k = 0; k < blobSize; k++)
        {
            rng = (rng * 1664525u) + 1013904223u;
            int dir = (int)(rng % 6);
            int nx = bx + ddx[dir];
            int ny = by + ddy[dir];
            int nz = bz + ddz[dir];
            if (nx < 1 || nx >= World::BLOCK_W - 1 || ny < 1 || ny >= World::BLOCK_H - 1 || nz < 1 || nz >= World::BLOCK_D - 1)
            {
                continue;
            }

            if (world.getBlock(nx, ny, nz) == BlockType::Stone)
            {
                world.setBlock(nx, ny, nz, ore);
            }

            bx = nx;
            by = ny;
            bz = nz;
        }
    }

    void generate(World &world, unsigned int seed)
    {
        for (int wx = 0; wx < World::BLOCK_W; wx++)
        {
            for (int wz = 0; wz < World::BLOCK_D; wz++)
            {
                bool isBorder = (wx == 0 || wx == World::BLOCK_W - 1 || wz == 0 || wz == World::BLOCK_D - 1);
                int surfaceY = computeSurfaceY((float)wx, (float)wz, seed);
                unsigned int soilH = (unsigned int)wx * 73856093 ^ (unsigned int)wz * 19349663 ^ (unsigned int)seed;
                int soilDepth = 2 + (int)(soilH % 2);
                for (int wy = 0; wy < World::BLOCK_H; wy++)
                {
                    if (isBorder || wy == 0)
                    {
                        world.setBlock(wx, wy, wz, BlockType::Bedrock);
                        continue;
                    }

                    BlockType t;
                    if (wy > surfaceY)
                    {
                        t = (wy <= oceanLevel + 1) ? BlockType::Water : BlockType::Air;
                    }
                    else if (wy == surfaceY)
                    {
                        t = (surfaceY > oceanLevel) ? BlockType::Turf : BlockType::Soil;
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

        for (int wx = 1; wx < World::BLOCK_W - 1; wx++)
        {
            for (int wz = 1; wz < World::BLOCK_D - 1; wz++)
            {
                int sy = computeSurfaceY((float) wx, (float) wz, seed);
                if (sy <= oceanLevel)
                {
                    float gritN = hashNoise(wx, wz, seed + 0x6B115u);
                    if (gritN > 0.3f)
                    {
                        if (sy >= 1)
                        {
                            world.setBlock(wx, sy,     wz, BlockType::Grit);
                        }

                        if (sy - 1 >= 1)
                        {
                            world.setBlock(wx, sy - 1, wz, BlockType::Grit);
                        }
                    }

                    continue;
                }

                float falloff = islandFalloff((float)wx, (float)wz);
                if (falloff < 0.22f && sy <= oceanLevel + 3)
                {
                    float siltN = hashNoise(wx, wz, seed + 0x5E115u);
                    if (siltN > 0.55f)
                    {
                        world.setBlock(wx, sy, wz, BlockType::Silt);
                        if (sy - 1 >= 1)
                        {
                            world.setBlock(wx, sy - 1, wz, BlockType::Silt);
                        }
                    }
                }
            }
        }

        for (int wx = 1; wx < World::BLOCK_W - 1; wx++)
        {
            for (int wz = 1; wz < World::BLOCK_D - 1; wz++)
            {
                int sy = computeSurfaceY((float)wx, (float)wz, seed);
                for (int wy = 2; wy < sy - 2; wy++)
                {
                    if (world.getBlock(wx, wy, wz) != BlockType::Soil)
                    {
                        continue;
                    }

                    bool exposed = (world.getBlock(wx - 1, wy, wz) == BlockType::Air ||
                                    world.getBlock(wx + 1, wy, wz) == BlockType::Air ||
                                    world.getBlock(wx, wy, wz - 1) == BlockType::Air ||
                                    world.getBlock(wx, wy, wz + 1) == BlockType::Air);
                    if (exposed)
                    {
                        world.setBlock(wx, wy, wz, BlockType::Stone);
                    }
                }
            }
        }

        for (int wx = 1; wx < World::BLOCK_W - 1; wx++)
        {
            for (int wz = 1; wz < World::BLOCK_D - 1; wz++)
            {
                int sy = computeSurfaceY((float)wx, (float)wz, seed);
                for (int wy = 2; wy <= sy - 4; wy++)
                {
                    auto depth = (float)(sy - wy);
                    float bias = std::clamp(depth / 32.f, 0.f, 1.f);
                    float threshold = 0.18f - (bias * 0.10f);
                    float n = fbm3((float)wx, (float)wy, (float)wz, seed + 0x9E3779B9u, 4);
                    if (std::abs(n) < threshold)
                    {
                        world.setBlock(wx, wy, wz, BlockType::Air);
                    }
                }
            }
        }

        unsigned int lavaSeed = seed + 0xDEADBEEFu;
        for (int wx = 1; wx < World::BLOCK_W - 1; wx++)
        {
            for (int wz = 1; wz < World::BLOCK_D - 1; wz++)
            {
                for (int wy = 2; wy < oceanLevel; wy++)
                {
                    if (world.getBlock(wx, wy, wz) != BlockType::Air)
                    {
                        continue;
                    }

                    float lava = fbm3((float)wx, (float)wy, (float)wz, lavaSeed);
                    if (lava > 0.30f)
                    {
                        world.setBlock(wx, wy, wz, BlockType::Lava);
                    }
                }
            }
        }

        struct OreSpec
        {
            BlockType type;
            int maxY;
            int centres;
            int blobSize;
            unsigned int seedOff;
        };

        OreSpec ores[] = {
                {BlockType::CharVein, 58, 200, 20, 0x0CA41u},
                {BlockType::IronVein, 45, 120, 20, 0x1A04Fu},
                {BlockType::GoldVein, 28,  60, 15, 0x60D6Eu},
        };
        for (auto &ore : ores)
        {
            unsigned int oreSeed = seed + ore.seedOff;
            for (int n = 0; n < ore.centres; n++)
            {
                unsigned int h = oreSeed ^ (unsigned int)(n * 2654435761u);
                h ^= h >> 16;
                h *= 0x45d9f3bu;
                h ^= h >> 16;
                int cx = 1 + (int)(h % (unsigned int)(World::BLOCK_W - 2));
                h ^= h >> 16;
                h *= 0x45d9f3bu;
                h ^= h >> 16;
                int cz = 1 + (int)(h % (unsigned int)(World::BLOCK_D - 2));
                h ^= h >> 16;
                h *= 0x45d9f3bu;
                h ^= h >> 16;
                int cy = 2 + (int)(h % (unsigned int)(ore.maxY - 2));
                if (world.getBlock(cx, cy, cz) != BlockType::Stone)
                {
                    continue;
                }

                int blobSize = (ore.blobSize / 2) + ((int)((h >> 8) % (unsigned int)((ore.blobSize / 2) + 1)));
                placeOreVein(world, cx, cy, cz, ore.type, blobSize, oreSeed + (unsigned int)n);
            }
        }

        unsigned int treeSeed = seed + 0xABCDEF01u;
        for (int wx = 2; wx < World::BLOCK_W - 2; wx++)
        {
            for (int wz = 2; wz < World::BLOCK_D - 2; wz++)
            {
                float treeN = hashNoise(wx, wz, treeSeed);
                if (treeN <= 0.96f)
                {
                    continue;
                }

                int sy = computeSurfaceY((float) wx, (float) wz, seed);
                if (sy <= oceanLevel)
                {
                    continue;
                }

                if (world.getBlock(wx, sy, wz) != BlockType::Turf)
                {
                    continue;
                }

                auto th = (unsigned int)(wx * 73856093 ^ wz * 19349663 ^ treeSeed);
                int height = 4 + (int) (th % 3);
                bool clear = true;
                for (int dy = 1; dy <= height + 2 && clear; dy++)
                {
                    int wy = sy + dy;
                    if (wy >= World::BLOCK_H)
                    {
                        break;
                    }

                    if (world.getBlock(wx, wy, wz) != BlockType::Air)
                    {
                        clear = false;
                    }
                }

                if (!clear)
                {
                    continue;
                }

                placeTree(world, wx, sy, wz, height);
            }
        }

        unsigned int plantSeed = seed + 0x50FA2301u;
        for (int wx = 2; wx < World::BLOCK_W - 2; wx++)
        {
            for (int wz = 2; wz < World::BLOCK_D - 2; wz++)
            {
                int sy = computeSurfaceY((float)wx, (float)wz, seed);
                if (sy > oceanLevel && world.getBlock(wx, sy, wz) == BlockType::Turf)
                {
                    int above = sy + 1;
                    if (above < World::BLOCK_H && world.getBlock(wx, above, wz) == BlockType::Air)
                    {
                        float flN = hashNoise(wx, wz, plantSeed);
                        if (flN > 0.97f)
                        {
                            BlockType flower = (hashNoise(wx + 1, wz, plantSeed) > 0.5f)
                                                       ? BlockType::Goldenbloom : BlockType::Thornbloom;
                            world.setBlock(wx, above, wz, flower);
                        }
                    }

                    continue;
                }

                for (int wy = 2; wy < sy - 6; wy++)
                {
                    if (world.getBlock(wx, wy, wz) != BlockType::Air)
                    {
                        continue;
                    }

                    if (wy < 1 || !blockDef(world.getBlock(wx, wy - 1, wz)).opaque)
                    {
                        continue;
                    }

                    if (wy + 1 < World::BLOCK_H && world.getBlock(wx, wy + 1, wz) != BlockType::Air)
                    {
                        continue;
                    }

                    if (hashNoise3(wx, wy, wz, plantSeed) > 0.992f)
                    {
                        BlockType shroom = (hashNoise3(wx + 1, wy, wz, plantSeed) > 0.5f)
                                                   ? BlockType::Dustshroom : BlockType::Emberscap;
                        world.setBlock(wx, wy, wz, shroom);
                        break;
                    }
                }
            }
        }
    }
}