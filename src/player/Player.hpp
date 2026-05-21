#ifndef CAVERN_PLAYER_HPP
#define CAVERN_PLAYER_HPP
#include <glm/glm.hpp>
#include "Physics.hpp"

class World;
class Input;

class Player
{
public:
    struct RayHit {
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
    RayHit hitBlock;

    [[nodiscard]] glm::vec3 eyePos() const { return position + glm::vec3(0.f, Physics::eye, 0.f); }

    void tick(float dt, World &world, Input &input);

    void respawn();

private:
    [[nodiscard]] RayHit castRay(const World &world) const;
};
#endif//CAVERN_PLAYER_HPP