#ifndef CAVERN_WORLDGEN_HPP
#define CAVERN_WORLDGEN_HPP
class World;

namespace WorldGen
{
    void generate(World &world, unsigned int seed, int activeExtent = 256);
}
#endif//CAVERN_WORLDGEN_HPP