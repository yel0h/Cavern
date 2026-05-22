#ifndef CAVERN_PARTICLEMANAGER_HPP
#define CAVERN_PARTICLEMANAGER_HPP
#include "src/world/Block.hpp"
#include <glm/glm.hpp>
#include <vector>

class World;

struct BlockParticle
{
    glm::vec3 pos;
    glm::vec3 vel;
    float r = 1.f;
    float g = 1.f;
    float b = 1.f;
    float age = 0.f;
    float groundTime = -1.f;
    bool grounded = false;
};

class ParticleManager
{
public:
    std::vector<BlockParticle> particles;

    void spawnFromBlock(int bx, int by, int bz, BlockType type);

    void tick(float dt, const World &world);
};
#endif//CAVERN_PARTICLEMANAGER_HPP