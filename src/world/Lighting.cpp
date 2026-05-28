#include "Lighting.hpp"
#include "World.hpp"

namespace Lighting
{
    void propagate(World &world)
    {
        for (int wx = 0; wx < World::BLOCK_W; wx++)
        {
            for (int wz = 0; wz < World::BLOCK_D; wz++)
            {
                bool lit = true;
                for (int wy = World::BLOCK_H - 1; wy >= 0; wy--)
                {
                    unsigned char lv = (wy <= 1) ? 0u : (lit ? 1u : 0u);
                    world.setLight(wx, wy, wz, lv);
                    if (blockDef(world.getBlock(wx, wy, wz)).opaque)
                    {
                        lit = false;
                    }
                }
            }
        }
    }

    void propagateColumn(World &world, int wx, int wz)
    {
        bool lit = true;
        for (int wy = World::BLOCK_H - 1; wy >= 0; wy--)
        {
            unsigned char lv = (wy <= 1) ? 0u : (lit ? 1u : 0u);
            world.setLight(wx, wy, wz, lv);
            if (blockDef(world.getBlock(wx, wy, wz)).opaque)
            {
                lit = false;
            }
        }
    }
}