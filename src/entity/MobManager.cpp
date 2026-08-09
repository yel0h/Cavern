#include "MobManager.hpp"
#include "../player/Player.hpp"
#include "../world/World.hpp"
#include "ParticleManager.hpp"
#include <algorithm>
#include <fstream>

static constexpr unsigned int mobMagic = 0x4D4F4243u;

static int meleeDamageFor(MobType t)
{
    static std::mt19937 mt{std::random_device()()};
    static std::uniform_int_distribution<int> dist(1, 5);
    switch (t)
    {
        case MobType::Boneshade:
        case MobType::Grubbin:
            return dist(mt);

        default:
            return 0;
    }
}

int MobManager::maxVitalityFor(MobType t)
{
    switch (t)
    {
        case MobType::Snout:
            return 8;

        case MobType::Boneshade:
        case MobType::Grubbin:
            return 10;

        case MobType::Fumewretch:
            return 6;
    }

    return 8;
}

void MobManager::pickWanderDir(Mob &m)
{
    float angle = angleDist(mt);
    m.dirX = std::cos(angle);
    m.dirZ = std::sin(angle);
    m.yaw = glm::degrees(std::atan2(m.dirX, -m.dirZ));
    static std::uniform_int_distribution<int> tickDist(40, 120);
    m.ticksLeft = tickDist(mt);
}

int MobManager::surfaceY(const World &world, int wx, int wz)
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

void MobManager::spawn(const World &world)
{
    mobs.clear();

    struct SpawnRule
    {
        MobType type;
        int count;
        bool underground;
    };

    const SpawnRule rules[4] = {
            {MobType::Snout, 24, false},
            {MobType::Boneshade, 16, false},
            {MobType::Grubbin, 16, false},
            {MobType::Fumewretch, 10, true},
    };
    static std::uniform_int_distribution<int> wDist(0, World::BLOCK_W - 1);
    static std::uniform_int_distribution<int> dDist(0, World::BLOCK_D - 1);
    for (const auto &rule : rules)
    {
        for (int i = 0; i < rule.count; i++)
        {
            int bx = wDist(mt);
            int bz = dDist(mt);
            int sy = surfaceY(world, bx, bz);
            Mob m;
            m.type = rule.type;
            m.vitality = maxVitalityFor(rule.type);
            m.ai = MobAI::Wander;
            if (!rule.underground)
            {
                m.position = {(float)bx + 0.5f, (float)sy, (float)bz + 0.5f};
            }
            else
            {
                bool placed = false;
                std::uniform_int_distribution<int> yDist(2, std::max(3, sy - 2));
                for (int tries = 0; tries < 6 && !placed; tries++)
                {
                    int wy = yDist(mt);
                    if (!blockDef(world.getBlock(bx, wy, bz)).opaque &&
                        wy > 0 && blockDef(world.getBlock(bx, wy - 1, bz)).opaque)
                    {
                        m.position = {(float)bx + 0.5f, (float)wy, (float)bz + 0.5f};
                        placed = true;
                    }
                }

                if (!placed)
                {
                    continue;
                }
            }

            m.frontLegPhase = angleDist(mt);
            m.rearLegPhase = angleDist(mt);
            m.light = (float)world.getLight(bx, (int)m.position.y, bz);
            pickWanderDir(m);
            mobs.push_back(m);
        }
    }
}

void MobManager::explode(Mob &m, World &world, ParticleManager &particles)
{
    constexpr int radius = 2;
    int cx = (int)std::floor(m.position.x);
    int cy = (int)std::floor(m.position.y);
    int cz = (int)std::floor(m.position.z);
    particles.spawnBurst(m.position, 0.85f, 0.35f, 0.10f, 48);
    for (int dx = -radius; dx <= radius; dx++)
    {
        for (int dy = -radius; dy <= radius; dy++)
        {
            for (int dz = -radius; dz <= radius; dz++)
            {
                if ((dx * dx) + (dy * dy) + (dz * dz) > (radius * radius) + 1)
                {
                    continue;
                }

                int wx = cx + dx;
                int wy = cy + dy;
                int wz = cz + dz;
                if (!World::inBounds(wx, wy, wz))
                {
                    continue;
                }

                BlockType t = world.getBlock(wx, wy, wz);
                if (t == BlockType::Air || t == BlockType::Bedrock)
                {
                    continue;
                }

                if (blockDef(t).liquid)
                {
                    continue;
                }

                world.setBlock(wx, wy, wz, BlockType::Air);
            }
        }
    }
}

void MobManager::tick(float dt, World &world, Player &player, ParticleManager &particles)
{
    constexpr float speed = 2.0f;
    constexpr float chaseSpeed = 2.6f;
    constexpr float aggroRadius = 10.f;
    constexpr float attackRange = 1.4f;
    constexpr float detonateRange = 1.6f;
    constexpr float burstRadius = 3.f;
    constexpr float attackInterval = 1.0f;
    for (auto it = mobs.begin(); it != mobs.end();)
    {
        Mob &m = *it;
        if (m.attackCooldown > 0.f)
        {
            m.attackCooldown -= dt;
        }

        bool hostile = (m.type != MobType::Snout);
        glm::vec3 toPlayer = glm::vec3(player.position.x, m.position.y, player.position.z) - m.position;
        float distToPlayer = glm::length(toPlayer);
        if (hostile)
        {
            if (distToPlayer < aggroRadius)
            {
                m.ai = MobAI::Chase;
            }
            else if (distToPlayer > aggroRadius * 1.5f)
            {
                m.ai = MobAI::Wander;
            }
        }

        float mSpeed = speed;
        if (m.ai == MobAI::Chase && distToPlayer > 0.01f)
        {
            glm::vec3 dir = glm::normalize(toPlayer);
            m.dirX = dir.x;
            m.dirZ = dir.z;
            m.yaw = glm::degrees(std::atan2(m.dirX, -m.dirZ));
            mSpeed = chaseSpeed;
        }
        else if (--m.ticksLeft <= 0)
        {
            pickWanderDir(m);
        }

        float nx = std::clamp(m.position.x + (m.dirX * mSpeed * dt), 0.5f, (float)World::BLOCK_W - 0.5f);
        float nz = std::clamp(m.position.z + (m.dirZ * mSpeed * dt), 0.5f, (float)World::BLOCK_D - 0.5f);
        int bx = (int)std::floor(nx);
        int bz = (int)std::floor(nz);
        int curY = (int)std::floor(m.position.y);
        bool airAtFeet = !blockDef(world.getBlock(bx, curY, bz)).opaque;
        bool groundBelow = (curY > 0) && blockDef(world.getBlock(bx, curY - 1, bz)).opaque;
        if (airAtFeet && groundBelow)
        {
            m.position.x = nx;
            m.position.z = nz;
        }
        else if (m.ai == MobAI::Wander)
        {
            pickWanderDir(m);
        }

        m.light = (float)world.getLight((int)std::floor(m.position.x), curY, (int)std::floor(m.position.z));
        if (hostile && m.type != MobType::Fumewretch && m.attackCooldown <= 0.f && distToPlayer < attackRange)
        {
            player.applyDamage(meleeDamageFor(m.type));
            m.attackCooldown = attackInterval;
        }

        if (m.type == MobType::Fumewretch && distToPlayer < detonateRange)
        {
            m.vitality = 0;
        }

        if (m.type == MobType::Fumewretch)
        {
            int maxV = maxVitalityFor(MobType::Fumewretch);
            if (m.vitality > 0 && m.vitality <= maxV / 3)
            {
                m.flashT = std::fmod(m.flashT + dt * 6.f, 1.f);
            }
            else
            {
                m.flashT = 0.f;
            }
        }

        if (m.vitality <= 0)
        {
            if (m.type == MobType::Fumewretch)
            {
                if (distToPlayer < burstRadius)
                {
                    float falloff = (burstRadius - distToPlayer) / burstRadius;
                    int burstDmg = (int)std::round(9.f * falloff);
                    player.applyDamage(burstDmg);
                }

                explode(m, world, particles);
            }

            it = mobs.erase(it);
            continue;
        }

        ++it;
    }
}

bool MobManager::attack(const glm::vec3 &eye, const glm::vec3 &forward, float reach, int damage, bool &killed, MobType &killedType)
{
    killed = false;
    Mob *best = nullptr;
    float bestDist = reach;
    for (auto &m : mobs)
    {
        glm::vec3 center = m.position + glm::vec3(0.f, 0.25f, 0.f);
        glm::vec3 toMob = center - eye;
        float dist = glm::length(toMob);
        if (dist > reach || dist < 0.001f)
        {
            continue;
        }

        glm::vec3 dir = toMob / dist;
        if (glm::dot(dir, forward) < 0.86f)
        {
            continue;
        }

        if (dist < bestDist)
        {
            bestDist = dist;
            best = &m;
        }
    }

    if (!best)
    {
        return false;
    }

    best->vitality -= damage;
    if (best->vitality <= 0)
    {
        killed = true;
        killedType = best->type;
    }

    return true;
}

bool MobManager::save(const char *path) const
{
    std::ofstream f(path, std::ios::binary);
    if (!f)
    {
        return false;
    }

    f.write(reinterpret_cast<char const *>(&mobMagic), sizeof(mobMagic));
    auto count = (unsigned int)mobs.size();
    f.write(reinterpret_cast<char const *>(&count), sizeof(count));
    for (const auto &m : mobs)
    {
        f.write(reinterpret_cast<char const *>(&m.position.x), sizeof(float));
        f.write(reinterpret_cast<char const *>(&m.position.y), sizeof(float));
        f.write(reinterpret_cast<char const *>(&m.position.z), sizeof(float));
        f.write(reinterpret_cast<char const *>(&m.yaw), sizeof(float));
        f.write(reinterpret_cast<char const *>(&m.dirX), sizeof(float));
        f.write(reinterpret_cast<char const *>(&m.dirZ), sizeof(float));
        auto type = (unsigned char)m.type;
        f.write(reinterpret_cast<char const *>(&type), sizeof(type));
        auto vit = (int)m.vitality;
        f.write(reinterpret_cast<char const *>(&vit), sizeof(vit));
    }

    return true;
}

bool MobManager::load(const char *path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        return false;
    }

    unsigned int magic = 0;
    f.read(reinterpret_cast<char *>(&magic), sizeof(magic));
    if (magic != mobMagic)
    {
        return false;
    }

    unsigned int count = 0;
    f.read(reinterpret_cast<char *>(&count), sizeof(count));
    if (count > 4096u)
    {
        return false;
    }

    mobs.clear();
    static std::uniform_int_distribution<int> dist(40, 119);
    for (unsigned int i = 0; i < count; i++)
    {
        Mob m{};
        f.read(reinterpret_cast<char *>(&m.position.x), sizeof(float));
        f.read(reinterpret_cast<char *>(&m.position.y), sizeof(float));
        f.read(reinterpret_cast<char *>(&m.position.z), sizeof(float));
        f.read(reinterpret_cast<char *>(&m.yaw), sizeof(float));
        f.read(reinterpret_cast<char *>(&m.dirX), sizeof(float));
        f.read(reinterpret_cast<char *>(&m.dirZ), sizeof(float));
        unsigned char type = 0;
        f.read(reinterpret_cast<char *>(&type), sizeof(type));
        m.type = (MobType)type;
        int vit = 0;
        f.read(reinterpret_cast<char *>(&vit), sizeof(vit));
        m.vitality = vit;
        m.ticksLeft = dist(mt);
        m.ai = MobAI::Wander;
        mobs.push_back(m);
    }

    return true;
}