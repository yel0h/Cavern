#include "WandererManager.hpp"
#include "../world/World.hpp"
#include <algorithm>

void WandererManager::pickDirection(Wanderer &w)
{
    float angle = angleDist(mt);
    w.dirX = std::cos(angle);
    w.dirZ = std::sin(angle);
    w.yaw = glm::degrees(std::atan2(-w.dirX, -w.dirZ));
    static std::uniform_int_distribution<int> dist(40, 120);
    w.ticksLeft = dist(mt);
}

int WandererManager::surfaceY(const World &world, int wx, int wz)
{
    for (int wy = World::BLOCK_H - 1; wy >= 0; wy--)
    {
        if (blockDef(world.getBlock(wx, wy, wz)).opaque)
        {
            return wy + 1;
        }
    }

    return 1;
}

void WandererManager::spawnOne(const World &world, float x, float z)
{
    static std::uniform_int_distribution<int> dist(-5, 5);
    int bx = (int)std::floor(x) + dist(mt);
    int bz = (int)std::floor(z) + dist(mt);
    bx = std::clamp(bx, 0, World::BLOCK_W - 1);
    bz = std::clamp(bz, 0, World::BLOCK_D - 1);
    int sy = surfaceY(world, bx, bz);
    Wanderer w;
    w.position.x = (float)bx + 0.5f;
    w.position.y = (float)sy;
    w.position.z = (float)bz + 0.5f;
    w.frontLegPhase = angleDist(mt);
    w.rearLegPhase = angleDist(mt);
    w.light = (float)world.getLight(bx, sy, bz);
    pickDirection(w);
    wanderers.push_back(w);
}

void WandererManager::tick(float dt, const World &world)
{
    constexpr float speed = 2.0f;
    wanderers.erase(
            std::remove_if(wanderers.begin(), wanderers.end(),
                           [](const Wanderer &w) { return w.position.y < -100.f; }),
            wanderers.end());
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
        int sy = surfaceY(world, bx, bz);
        bool airAtFeet = !blockDef(world.getBlock(bx, sy, bz)).opaque;
        bool groundBelow = (sy > 0) && blockDef(world.getBlock(bx, sy - 1, bz)).opaque;
        if (airAtFeet && groundBelow)
        {
            w.position.x = nx;
            w.position.y = (float)sy;
            w.position.z = nz;
        }
        else
        {
            pickDirection(w);
        }

        w.light = (float)world.getLight(bx, sy, bz);
    }
}