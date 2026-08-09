#include "ItemDropManager.hpp"
#include "../world/World.hpp"
#include <algorithm>
#include <random>

static void dropColor(BlockType t, float &r, float &g, float &b)
{
    switch (t)
    {
        case BlockType::Turf:
            r = 60 / 255.f;
            g = 145 / 255.f;
            b = 40 / 255.f;
            break;

        case BlockType::Stone:
            r = 112 / 255.f;
            g = 116 / 255.f;
            b = 134 / 255.f;
            break;

        case BlockType::Rubble:
            r = 100 / 255.f;
            g = 100 / 255.f;
            b = 100 / 255.f;
            break;

        case BlockType::Soil:
            r = 124 / 255.f;
            g = 68 / 255.f;
            b = 20 / 255.f;
            break;

        case BlockType::Timber:
            r = 168 / 255.f;
            g = 112 / 255.f;
            b = 48 / 255.f;
            break;

        case BlockType::Boards:
            r = 190 / 255.f;
            g = 150 / 255.f;
            b = 90 / 255.f;
            break;

        case BlockType::Sapling:
            r = 70 / 255.f;
            g = 150 / 255.f;
            b = 60 / 255.f;
            break;

        case BlockType::CharVein:
            r = 180 / 255.f;
            g = 80 / 255.f;
            b = 40 / 255.f;
            break;

        case BlockType::IronVein:
            r = 120 / 255.f;
            g = 120 / 255.f;
            b = 130 / 255.f;
            break;

        case BlockType::GoldVein:
            r = 210 / 255.f;
            g = 175 / 255.f;
            b = 60 / 255.f;
            break;

        default:
            r = 1.f;
            g = 1.f;
            b = 1.f;
            break;
    }
}

void ItemDropManager::spawn(glm::vec3 pos, BlockType type, int count)
{
    ItemDrop d;
    d.position = pos;
    static std::mt19937 mt{std::random_device()()};
    static std::uniform_real_distribution<float> xzDist(-1.5f, 1.5f);
    static std::uniform_real_distribution<float> yDist(2.f, 3.5f);
    d.velocity = {xzDist(mt), yDist(mt), xzDist(mt)};
    d.type = type;
    d.count = count;
    dropColor(type, d.r, d.g, d.b);
    drops.push_back(d);
}

void ItemDropManager::tick(float dt, const World &world, glm::vec3 playerPos)
{
    constexpr float gravity = -20.f;
    constexpr float maxAge = 60.f;
    constexpr float pickupRadius = 1.2f;
    constexpr float pickupDelay = 0.4f;
    for (auto &d : drops)
    {
        d.age += dt;
        if (!d.grounded)
        {
            d.velocity.y += gravity * dt;
            d.position += d.velocity * dt;
            int bx = (int)std::floor(d.position.x);
            int by = (int)std::floor(d.position.y);
            int bz = (int)std::floor(d.position.z);
            bool hit = d.position.y < 0.f;
            if (!hit && World::inBounds(bx, by, bz) && blockDef(world.getBlock(bx, by, bz)).opaque)
            {
                d.position.y = (float)by + 1.f;
                hit = true;
            }

            if (hit)
            {
                d.grounded = true;
                d.velocity = {0.f, 0.f, 0.f};
            }
        }

        if (!d.pickedUp && d.age > pickupDelay && glm::length(d.position - playerPos) < pickupRadius)
        {
            pending.push_back({d.type, d.count});
            d.pickedUp = true;
        }
    }

    drops.erase(
            std::remove_if(drops.begin(), drops.end(),
                           [](const ItemDrop &d)
                           { return d.pickedUp || d.age > maxAge; }),
            drops.end());
}

std::vector<PickupEvent> ItemDropManager::drainPickups()
{
    auto out = std::move(pending);
    pending.clear();
    return out;
}