#ifndef CAVERN_BOLT_HPP
#define CAVERN_BOLT_HPP
#include <glm/glm.hpp>

struct Bolt
{
    glm::vec3 position{0.f};
    glm::vec3 velocity{0.f};
    float age = 0.f;
};
#endif//CAVERN_BOLT_HPP