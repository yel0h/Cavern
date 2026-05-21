#ifndef CAVERN_WORLDGEN_HPP
#define CAVERN_WORLDGEN_HPP
class World;

namespace WorldGen
{
    void generate(World &world, unsigned int seed);

    void turfPass(World &world);

    void cavePass(World &world, unsigned int seed);
}
#endif//CAVERN_WORLDGEN_HPP