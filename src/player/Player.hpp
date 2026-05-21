#ifndef CAVERN_PLAYER_HPP
#define CAVERN_PLAYER_HPP
#include <glm/glm.hpp>
#include "Physics.hpp"

class World;
class Input;

class Player
{
private:
    glm::vec3 pendingMove{0.f};

public:
    glm::vec3 position{128.f, 74.f, 128.f};
    glm::vec3 velocity{0.f, 0.f, 0.f};
    float yaw = 0.f;
    float pitch = 0.f;
    bool onGround = false;

    [[nodiscard]] glm::vec3 eyePos() const { return position + glm::vec3(0.f, Physics::eye, 0.f); }

    void tick(float dt, const World &world, const Input &input);

    void respawn();
};
#endif//CAVERN_PLAYER_HPP