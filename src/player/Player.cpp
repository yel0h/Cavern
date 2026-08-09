#include "Player.hpp"
#include "../core/Input.hpp"
#include "../world/World.hpp"
#include "src/world/Lighting.hpp"
#include <algorithm>
#include <random>

bool Player::overlapsPlayer(int bx, int by, int bz) const
{
    constexpr float halfW = Physics::width * 0.5f;
    float fx = position.x;
    float fy = position.y;
    float fz = position.z;
    return (fx - halfW < bx + 1 && fx + halfW > bx && fy < by + 1 && fy + Physics::height > by && fz - halfW < bz + 1 && fz + halfW > bz);
}

bool Player::isBlockSolid(const World &world, int wx, int wy, int wz)
{
    if (wy < 0)
    {
        return false;
    }

    return blockDef(world.getBlock(wx, wy, wz)).opaque;
}

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

Player::RayHit Player::castRay(const World &world) const
{
    glm::vec3 eye = eyePos();
    float yr = glm::radians(yaw);
    float pr = glm::radians(pitch);
    glm::vec3 dir{std::cos(pr) * std::sin(yr), std::sin(pr), -std::cos(pr) * std::cos(yr)};
    int ix = (int)std::floor(eye.x);
    int iy = (int)std::floor(eye.y);
    int iz = (int)std::floor(eye.z);
    int stepX = (dir.x >= 0.f) ? 1 : -1;
    int stepY = (dir.y >= 0.f) ? 1 : -1;
    int stepZ = (dir.z >= 0.f) ? 1 : -1;
    float tDeltaX = (dir.x != 0.f) ? std::abs(1.f / dir.x) : 1e30f;
    float tDeltaY = (dir.y != 0.f) ? std::abs(1.f / dir.y) : 1e30f;
    float tDeltaZ = (dir.z != 0.f) ? std::abs(1.f / dir.z) : 1e30f;
    float tMaxX = (dir.x != 0.f) ? std::abs(((stepX > 0 ? std::floor(eye.x) + 1.f : std::floor(eye.x)) - eye.x) / dir.x) : 1e30f;
    float tMaxY = (dir.y != 0.f) ? std::abs(((stepY > 0 ? std::floor(eye.y) + 1.f : std::floor(eye.y)) - eye.y) / dir.y) : 1e30f;
    float tMaxZ = (dir.z != 0.f) ? std::abs(((stepZ > 0 ? std::floor(eye.z) + 1.f : std::floor(eye.z)) - eye.z) / dir.z) : 1e30f;
    int prevX = ix;
    int prevY = iy;
    int prevZ = iz;
    constexpr float maxReach = 6.0f;
    bool firstStep = true;
    while (true)
    {
        if (!firstStep && World::inBounds(ix, iy, iz) && blockDef(world.getBlock(ix, iy, iz)).opaque)
        {
            return {true, ix, iy, iz, prevX, prevY, prevZ};
        }

        firstStep = false;
        prevX = ix;
        prevY = iy;
        prevZ = iz;
        if (tMaxX <= tMaxY && tMaxX <= tMaxZ)
        {
            if (tMaxX > maxReach)
            {
                break;
            }

            ix += stepX;
            tMaxX += tDeltaX;
        }
        else if (tMaxY <= tMaxZ)
        {
            if (tMaxY > maxReach)
            {
                break;
            }

            iy += stepY;
            tMaxY += tDeltaY;
        }
        else
        {
            if (tMaxZ > maxReach)
            {
                break;
            }

            iz += stepZ;
            tMaxZ += tDeltaZ;
        }
    }

    return {};
}

void Player::applyMouseLook(const Input &input)
{
    constexpr float sensitivity = 0.12f;
    yaw += input.mouseDX * sensitivity;
    float pitchDelta = input.mouseDY * sensitivity;
    pitch += input.invertY ? pitchDelta : -pitchDelta;
    pitch = std::clamp(pitch, -89.f, 89.f);
}

void Player::tick(float dt, World &world, Input &input)
{
    int fx = (int)std::floor(position.x);
    int fy = (int)std::floor(position.y);
    int fz = (int)std::floor(position.z);
    int ex = (int)std::floor(position.x);
    int ey = (int)std::floor(position.y + Physics::eye);
    int ez = (int)std::floor(position.z);
    BlockType feetBlock = World::inBounds(fx, fy, fz) ? world.getBlock(fx, fy, fz) : BlockType::Air;
    BlockType eyeBlock = World::inBounds(ex, ey, ez) ? world.getBlock(ex, ey, ez) : BlockType::Air;
    underLava = isLavaLike(eyeBlock);
    underWater = isWaterLike(eyeBlock);
    bool inLiquid = blockDef(feetBlock).liquid;
    if (isDown)
    {
        return;
    }

    if (underWater)
    {
        breath = std::max(0.f, breath - dt);
        if (breath <= 0.f)
        {
            drownTimer += dt;
            constexpr float drownInterval = 0.6f;
            if (drownTimer >= drownInterval)
            {
                drownTimer -= drownInterval;
                applyDamage(1);
            }
        }
    }
    else
    {
        breath = std::min(6.f, breath + (dt * 2.f));
        drownTimer = 0.f;
    }

    if (isBlockSolid(world, fx, fy, fz))
    {
        position.y += 5.0f;
    }

    hitBlock = castRay(world);
    lastBroken = {};
    lastPlaced = {};
    placeMode = input.switchHeld;
    if (hitBlock.valid && input.switchMode)
    {
        int px = hitBlock.px;
        int py = hitBlock.py;
        int pz = hitBlock.pz;
        BlockType toPlace = (selectedBlock == BlockType::Turf) ? BlockType::Soil : selectedBlock;
        BlockType existing = world.getBlock(px, py, pz);
        if ((toPlace != BlockType::Bedrock || isWarden)
            && World::inBounds(px, py, pz)
            && (existing == BlockType::Air || blockDef(existing).liquid)
            && !overlapsPlayer(px, py, pz))
        {
            world.setBlock(px, py, pz, toPlace);
            Lighting::propagateColumn(world, px, pz);
            lastPlaced = {true, px, py, pz, toPlace};
        }
    }

    if (hitBlock.valid && input.primaryAction)
    {
        BlockType target = world.getBlock(hitBlock.bx, hitBlock.by, hitBlock.bz);
        if (target != BlockType::Bedrock || isWarden)
        {
            lastBroken = {true, hitBlock.bx, hitBlock.by, hitBlock.bz, target};
            world.setBlock(hitBlock.bx, hitBlock.by, hitBlock.bz, BlockType::Air);
            Lighting::propagateColumn(world, hitBlock.bx, hitBlock.bz);
        }
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

    if (inLiquid)
    {
        velocity.x = move.x;
        velocity.z = move.z;
        velocity.y += Physics::gravity * 0.5f * dt;
        if (input.jump)
        {
            velocity.y = 8.f;
        }
    }
    else
    {
        if (onGround)
        {
            velocity.x = move.x;
            velocity.z = move.z;
        }
        else
        {
            velocity.x += (move.x - velocity.x) * Physics::airControl;
            velocity.z += (move.z - velocity.z) * Physics::airControl;
        }

        if (input.getJumpPressed() && onGround)
        {
            velocity.y = Physics::jump;
            onGround = false;
        }

        velocity.y = std::max(velocity.y + (Physics::gravity * dt), Physics::terminalVelocity);
    }

    bool prevOnGround = onGround;
    if (prevOnGround)
    {
        fallPeakY = position.y;
    }
    else
    {
        fallPeakY = std::max(fallPeakY, position.y);
    }

    onGround = false;
    resolveAABB(position, velocity * dt, world, onGround);
    if (onGround)
    {
        velocity.y = 0.f;
    }

    justLanded = (!prevOnGround && onGround);
    footstepReady = false;
    if (justLanded)
    {
        constexpr float safeFall = 4.f;
        float fallDist = fallPeakY - position.y;
        if (fallDist > safeFall)
        {
            int dmg = (int)std::floor(fallDist - safeFall) + 1;
            applyDamage(dmg);
        }
    }

    damageFlashT = std::max(0.f, damageFlashT - dt);
    float hspd = std::sqrt((velocity.x * velocity.x) + (velocity.z * velocity.z));
    if (onGround && hspd > 2.f)
    {
        if (--footstepTimer <= 0)
        {
            footstepTimer = 18;
            footstepReady = true;
        }
    }
    else
    {
        footstepTimer = 0;
    }

    int fsbx = (int)std::floor(position.x);
    int fsby = (int)std::floor(position.y) - 1;
    int fsbz = (int)std::floor(position.z);
    blockBelow = World::inBounds(fsbx, fsby, fsbz) ? world.getBlock(fsbx, fsby, fsbz) : BlockType::Bedrock;
}

void Player::respawn()
{
    position = {spawnX, 45.f, spawnZ};
    velocity = {0.f, 0.f, 0.f};
    onGround = false;
    vitality = maxVitality;
    isDown = false;
    damageFlashT = 0.f;
    breath = 6.f;
    drownTimer = 0.f;
    fallPeakY = position.y;
}

void Player::applyDamage(int amount)
{
    if (isDown || amount <= 0)
    {
        return;
    }

    vitality = std::max(0, vitality - amount);
    damageFlashT = 0.35f;
    if (vitality <= 0)
    {
        isDown = true;
    }
}

void Player::resetSpawn()
{
    spawnX = 128.f;
    spawnZ = 128.f;
}

void Player::setSpawn()
{
    spawnX = position.x;
    spawnZ = position.z;
}

void Player::loadSpawn(float x, float z)
{
    spawnX = x;
    spawnZ = z;
}