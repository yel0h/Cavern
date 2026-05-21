#include "Player.hpp"
#include "../core/Input.hpp"
#include "../world/World.hpp"
#include <algorithm>
#include <random>

static float halfW() { return Physics::width * 0.5f; }

static void resolveAABB(glm::vec3 &pos, glm::vec3 delta, const World &world, bool &onGround)
{
    auto solid = [&](int wx, int wy, int wz) -> bool
    {
        if (wy < 0)
        {
            return false;
        }

        return blockDef(world.getBlock(wx, wy, wz)).opaque;
    };
    float hw = halfW();
    float h = Physics::height;
    pos.x += delta.x;
    {
        int minY = (int)std::floor(pos.y);
        int maxY = (int)std::floor(pos.y + h - 0.001f);
        int minZ = (int)std::floor(pos.z - hw);
        int maxZ = (int)std::floor(pos.z + hw - 0.001f);
        if (delta.x > 0)
        {
            int bx = (int)std::floor(pos.x + hw);
            bool hit = false;
            for (int by = minY; by <= maxY && !hit; by++)
            {
                for (int bz = minZ; bz <= maxZ && !hit; bz++)
                {
                    if (solid(bx, by, bz))
                    {
                        hit = true;
                    }
                }
            }

            if (hit)
            {
                pos.x = bx - hw;
            }
        }
        else if (delta.x < 0)
        {
            int bx = (int)std::floor(pos.x - hw);
            bool hit = false;
            for (int by = minY; by <= maxY && !hit; by++)
            {
                for (int bz = minZ; bz <= maxZ && !hit; bz++)
                {
                    if (solid(bx, by, bz))
                    {
                        hit = true;
                    }
                }
            }

            if (hit)
            {
                pos.x = bx + 1.f + hw;
            }
        }
    }

    pos.z += delta.z;
    {
        int minY = (int)std::floor(pos.y);
        int maxY = (int)std::floor(pos.y + h - 0.001f);
        int minX = (int)std::floor(pos.x - hw);
        int maxX = (int)std::floor(pos.x + hw - 0.001f);
        if (delta.z > 0)
        {
            int bz = (int)std::floor(pos.z + hw);
            bool hit = false;
            for (int by = minY; by <= maxY && !hit; by++)
            {
                for (int bx = minX; bx <= maxX && !hit; bx++)
                {
                    if (solid(bx, by, bz))
                    {
                        hit = true;
                    }
                }
            }

            if (hit)
            {
                pos.z = bz - hw;
            }
        }
        else if (delta.z < 0)
        {
            int bz = (int)std::floor(pos.z - hw);
            bool hit = false;
            for (int by = minY; by <= maxY && !hit; by++)
            {
                for (int bx = minX; bx <= maxX && !hit; bx++)
                {
                    if (solid(bx, by, bz))
                    {
                        hit = true;
                    }
                }
            }

            if (hit)
            {
                pos.z = bz + 1.f + hw;
            }
        }
    }

    pos.y += delta.y;
    {
        int minX = (int)std::floor(pos.x - hw);
        int maxX = (int)std::floor(pos.x + hw - 0.001f);
        int minZ = (int)std::floor(pos.z - hw);
        int maxZ = (int)std::floor(pos.z + hw - 0.001f);
        if (delta.y > 0)
        {
            int by = (int)std::floor(pos.y + h);
            bool hit = false;
            for (int bx = minX; bx <= maxX && !hit; bx++)
            {
                for (int bz = minZ; bz <= maxZ && !hit; bz++)
                {
                    if (solid(bx, by, bz))
                    {
                        hit = true;
                    }
                }
            }

            if (hit)
            {
                pos.y = by - h;
            }
        }
        else if (delta.y < 0)
        {
            int by = (int)std::floor(pos.y);
            bool hit = false;
            for (int bx = minX; bx <= maxX && !hit; bx++)
            {
                for (int bz = minZ; bz <= maxZ && !hit; bz++)
                {
                    if (solid(bx, by, bz))
                    {
                        hit = true;
                    }
                }
            }

            if (hit)
            {
                pos.y = by + 1.f;
                onGround = true;
            }
        }
    }
}

void Player::tick(float dt, const World &world, const Input &input)
{
    yaw += input.mouseDX * 0.1f;
    pitch -= input.mouseDY * 0.1f;
    pitch = std::clamp(pitch, -89.f, 89.f);
    if (input.respawn)
    {
        respawn();
        return;
    }

    float cy = std::cos(glm::radians(yaw));
    float sy = std::sin(glm::radians(yaw));
    glm::vec3 fwd{sy, 0.f, -cy};
    glm::vec3 right{cy, 0.f, sy};
    glm::vec3 move{0.f};
    if (input.forward)
    {
        move += fwd;
    }

    if (input.backward)
    {
        move -= fwd;
    }

    if (input.right)
    {
        move += right;
    }

    if (input.left)
    {
        move -= right;
    }

    if (glm::length(move) > 0.001f)
    {
        move = glm::normalize(move) * Physics::walk;
    }

    velocity.x = move.x;
    velocity.z = move.z;
    if (input.jump && onGround)
    {
        velocity.y = Physics::jump;
        onGround = false;
    }

    velocity.y += Physics::gravity * dt;
    onGround = false;
    resolveAABB(position, velocity * dt, world, onGround);
    if (onGround)
    {
        velocity.y = 0.f;
    }
}

void Player::respawn()
{
    static std::mt19937 mt{std::random_device()()};
    static std::uniform_real_distribution<float> dist(0.5f, World::BLOCK_W - 0.5f);
    float rx = dist(mt);
    float rz = dist(mt);
    position = {rx, 74.f, rz};
    velocity = {0.f, 0.f, 0.f};
    onGround = false;
}