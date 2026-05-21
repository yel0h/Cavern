#ifndef CAVERN_LIGHTING_HPP
#define CAVERN_LIGHTING_HPP
class World;

namespace Lighting
{
    void propagate(World &world);

    void propagateColumn(World &world, int wx, int wz);
}
#endif//CAVERN_LIGHTING_HPP