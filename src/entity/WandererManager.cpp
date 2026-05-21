#include "WandererManager.hpp"
#include "../world/World.hpp"
#include <algorithm>

void WandererManager::pickDirection(Wanderer &w)
{
    float angle = angleDist(mt);
    w.dirX = std::cos(angle);
    w.dirZ = std::sin(angle);
    w.yaw = glm::degrees(std::atan2(w.dirX, w.dirZ));
    static std::uniform_int_distribution<int> dist(40, 120);
    w.ticksLeft  = dist(mt);
}

void WandererManager::spawn(const World &world)
{
    wanderers.clear();
    wanderers.reserve(count);
    static std::uniform_real_distribution<float> wDist(0.5f, World::BLOCK_W - 0.5f);
    static std::uniform_real_distribution<float> dDist(0.5f, World::BLOCK_D - 0.5f);
    for (int i = 0; i < count; i++)
    {
        Wanderer w;
        w.position.x = wDist(mt);
        w.position.y = 44.f;
        w.position.z = dDist(mt);
        w.leftArmPhase = angleDist(mt);
        w.rightArmPhase = angleDist(mt);
        w.light = (float)world.getLight((int)w.position.x, 44, (int)w.position.z);
        pickDirection(w);
        wanderers.push_back(w);
    }
}

void WandererManager::tick(float dt, const World &world)
{
    constexpr float speed = 2.0f;
    for (auto &w : wanderers)
    {
        if (--w.ticksLeft <= 0)
        {
            pickDirection(w);
        }

        float nx = w.position.x + (w.dirX * speed * dt);
        float nz = w.position.z + (w.dirZ * speed * dt);
        nx = std::clamp(nx, 0.5f, (float)World::BLOCK_W - 0.5f);
        nz = std::clamp(nz, 0.5f, (float)World::BLOCK_D - 0.5f);
        int bx = (int)std::floor(nx);
        int bz = (int)std::floor(nz);
        bool airAhead = !blockDef(world.getBlock(bx, 44, bz)).opaque;
        bool groundBelow = blockDef(world.getBlock(bx, 43, bz)).opaque;
        if (airAhead && groundBelow)
        {
            w.position.x = nx;
            w.position.z = nz;
        }
        else
        {
            pickDirection(w);
        }

        w.light = (float)world.getLight(bx, 44, bz);
    }
}