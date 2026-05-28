#ifndef CAVERN_PLAYER_HPP
#define CAVERN_PLAYER_HPP
#include "Physics.hpp"
#include "src/world/Block.hpp"
#include <glm/glm.hpp>

class World;
class Input;

class Player
{
public:
    struct RayHit
    {
        bool valid = false;
        int bx = 0;
        int by = 0;
        int bz = 0;
        int face = 0;
        int px = 0;
        int py = 0;
        int pz = 0;
    };

    glm::vec3 position{128.f, 45.f, 128.f};
    glm::vec3 velocity{0.f, 0.f, 0.f};
    float yaw = 0.f;
    float pitch = 0.f;
    bool onGround = false;

    struct BrokenEvent
    {
        bool valid = false;
        int bx = 0;
        int by = 0;
        int bz = 0;
        BlockType type = BlockType::Air;
    };

    RayHit hitBlock;
    BrokenEvent lastBroken;
    BlockType selectedBlock = BlockType::Stone;
    bool placeMode = true;
    bool underLava = false;

    [[nodiscard]] glm::vec3 eyePos() const { return position + glm::vec3(0.f, Physics::eye, 0.f); }

    void tick(float dt, World &world, Input &input);

    void applyMouseLook(const Input &input);

    void respawn();

private:
    [[nodiscard]] static bool isBlockSolid(const World &world, int wx, int wy, int wz) ;

    [[nodiscard]] RayHit castRay(const World &world) const;

    bool overlapsPlayer(int bx, int by, int bz) const;
};
#endif//CAVERN_PLAYER_HPP