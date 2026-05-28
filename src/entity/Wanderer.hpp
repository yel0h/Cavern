#ifndef CAVERN_WANDERER_HPP
#define CAVERN_WANDERER_HPP
#include <glm/glm.hpp>

struct Wanderer
{
    glm::vec3 position;
    float yaw = 0.f;
    float dirX = 0.f;
    float dirZ = 0.f;
    int ticksLeft = 0;
    float frontLegPhase = 0.f;
    float rearLegPhase = 0.f;
    float light = 1.f;
};
#endif//CAVERN_WANDERER_HPP