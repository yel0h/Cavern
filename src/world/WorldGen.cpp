#include "WorldGen.hpp"
#include "World.hpp"

namespace WorldGen
{
    void generate(World &world, unsigned int)
    {
        for (int wx = 0; wx < World::BLOCK_W; wx++)
        {
            for (int wz = 0; wz < World::BLOCK_D; wz++)
            {
                for (int wy = 0; wy < World::BLOCK_H; wy++)
                {
                    BlockType t = BlockType::Air;
                    if (wy < 43)
                    {
                        t = BlockType::Stone;
                    }
                    else if (wy == 43)
                    {
                        t = BlockType::Turf;
                    }

                    world.setBlock(wx, wy, wz, t);
                }
            }
        }
    }
}