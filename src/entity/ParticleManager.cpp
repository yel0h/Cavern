#include "ParticleManager.hpp"
#include "../world/World.hpp"
#include <algorithm>

static void blockColor(BlockType type, float &r, float &g, float &b)
{
    switch (type)
    {
        case BlockType::Turf:
            r = 60/255.f;
            g = 145/255.f;
            b = 40/255.f;
            break;

        case BlockType::Stone:
            r = 112/255.f;
            g = 116/255.f;
            b = 134/255.f;
            break;

        case BlockType::Rubble:
            r = 100/255.f;
            g = 100/255.f;
            b = 100/255.f;
            break;

        case BlockType::Soil:
            r = 124/255.f;
            g = 68/255.f;
            b = 20/255.f;
            break;

        case BlockType::Timber:
            r = 168/255.f;
            g = 112/255.f;
            b = 48/255.f;
            break;

        default:
            r = 1.f;
            g = 1.f;
            b = 1.f;
            break;
    }
}

void ParticleManager::spawnFromBlock(int bx, int by, int bz, BlockType type)
{
    float r, g, b;
    blockColor(type, r, g, b);
    static std::uniform_real_distribution<float> posDist(0.f, 1.f);
    static std::uniform_real_distribution<float> velDist(-4.f, 4.f);
    static std::uniform_real_distribution<float> yVelDist(1.f, 9.f);
    for (int i = 0; i < 64; i++)
    {
        BlockParticle p;
        p.pos = {(float)bx + posDist(mt), (float)by + posDist(mt), (float)bz + posDist(mt)};
        p.vel = {velDist(mt), yVelDist(mt), velDist(mt)};
        p.r = std::clamp(r + colorDist(mt), 0.f, 1.f);
        p.g = std::clamp(g + colorDist(mt), 0.f, 1.f);
        p.b = std::clamp(b + colorDist(mt), 0.f, 1.f);
        particles.push_back(p);
    }
}

void ParticleManager::spawnBurst(glm::vec3 center, float r, float g, float b, int count)
{
    static std::uniform_real_distribution<float> speedDist(3.f, 9.f);
    static std::uniform_real_distribution<float> thetaDist(0.f, 6.283f);
    static std::uniform_real_distribution<float> phiDist(0.f, 3.14159f);
    for (int i = 0; i < count; i++)
    {
        BlockParticle p;
        p.pos = center;
        float speed = speedDist(mt);
        float theta = thetaDist(mt);
        float phi = phiDist(mt);
        p.vel = {speed * std::sin(phi) * std::cos(theta), speed * std::cos(phi) + 3.f, speed * std::sin(phi) * std::sin(theta)};
        p.r = std::clamp(r + colorDist(mt), 0.f, 1.f);
        p.g = std::clamp(g + colorDist(mt), 0.f, 1.f);
        p.b = std::clamp(b + colorDist(mt), 0.f, 1.f);
        particles.push_back(p);
    }
}

void ParticleManager::tick(float dt, const World &world)
{
    constexpr float gravity = -20.f;
    constexpr float maxAge = 8.f;
    constexpr float groundTTL = 1.5f;
    for (auto &p : particles)
    {
        p.age += dt;
        if (!p.grounded)
        {
            p.vel.y += gravity * dt;
            p.pos += p.vel * dt;
            int bx = (int)std::floor(p.pos.x);
            int by = (int)std::floor(p.pos.y);
            int bz = (int)std::floor(p.pos.z);
            bool hit = p.pos.y < 0.f;
            if (!hit && World::inBounds(bx, by, bz) && blockDef(world.getBlock(bx, by, bz)).opaque)
            {
                p.pos.y = (float)by + 1.f;
                hit = true;
            }

            if (hit)
            {
                p.grounded = true;
                p.vel = {0.f, 0.f, 0.f};
                p.groundTime = p.age;
            }
        }
    }

    particles.erase(std::remove_if(particles.begin(), particles.end(),[](const BlockParticle &p)
                                   {
                                       if (p.grounded)
                                       {
                                           return (p.age - p.groundTime) > groundTTL;
                                       }

                                       return p.age > maxAge;
                                   }), particles.end());
}