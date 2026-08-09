#ifndef CAVERN_MOB_HPP
#define CAVERN_MOB_HPP
#include <glm/glm.hpp>

enum class MobType : unsigned char
{
    Snout,
    Boneshade,
    Grubbin,
    Fumewretch
};

enum class MobAI : unsigned char
{
    Wander,
    Chase
};

struct Mob
{
    glm::vec3 position{0.f};
    float yaw = 0.f;
    float dirX = 0.f;
    float dirZ = 0.f;
    int ticksLeft = 0;
    float frontLegPhase = 0.f;
    float rearLegPhase = 0.f;
    float light = 1.f;
    MobType type = MobType::Snout;
    MobAI ai = MobAI::Wander;
    int vitality = 1;
    float attackCooldown = 0.f;
    float flashT = 0.f;
};
#endif//CAVERN_MOB_HPP