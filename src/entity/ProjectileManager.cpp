#include "ProjectileManager.hpp"
#include "MobManager.hpp"
#include "../world/World.hpp"

void ProjectileManager::spawn(glm::vec3 pos, glm::vec3 dir)
{
    constexpr float boltSpeed = 24.f;
    Bolt b;
    b.position = pos;
    b.velocity = glm::normalize(dir) * boltSpeed;
    bolts.push_back(b);
}

void ProjectileManager::tick(float dt, World &world, MobManager &mobs, std::vector<MobType> &kills)
{
    constexpr float lifetime = 0.8f;
    constexpr int boltDamage = 3;
    constexpr float hitRadius = 0.6f;
    for (auto it = bolts.begin(); it != bolts.end();)
    {
        Bolt &b = *it;
        b.age += dt;
        glm::vec3 newPos = b.position + (b.velocity * dt);
        bool destroyed = false;
        int bx = (int)std::floor(newPos.x);
        int by = (int)std::floor(newPos.y);
        int bz = (int)std::floor(newPos.z);
        if (World::inBounds(bx, by, bz) && blockDef(world.getBlock(bx, by, bz)).opaque)
        {
            destroyed = true;
        }

        if (!destroyed)
        {
            bool killed = false;
            MobType kt{};
            if (mobs.damageMobAt(newPos, hitRadius, boltDamage, killed, kt))
            {
                destroyed = true;
                if (killed)
                {
                    kills.push_back(kt);
                }
            }
        }

        b.position = newPos;
        if (destroyed || b.age > lifetime)
        {
            it = bolts.erase(it);
        }
        else
        {
            ++it;
        }
    }
}