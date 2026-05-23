#ifndef CAVERN_WANDERERMANAGER_HPP
#define CAVERN_WANDERERMANAGER_HPP
#include "Wanderer.hpp"
#include <random>
#include <vector>

class World;

class WandererManager
{
private:
    std::mt19937 mt{std::random_device()()};
    std::uniform_real_distribution<float> angleDist{0, 6.28f};

    void pickDirection(Wanderer &w);

    static int surfaceY(const World &world, int wx, int wz) ;

public:
    static constexpr int count = 100;
    std::vector<Wanderer> wanderers;

    void spawn(const World &world);

    void spawnOne(const World &world, float x, float z);

    void tick(float dt, const World &world);
};
#endif//CAVERN_WANDERERMANAGER_HPP