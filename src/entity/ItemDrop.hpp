#ifndef CAVERN_ITEMDROP_HPP
#define CAVERN_ITEMDROP_HPP
#include "src/world/Block.hpp"
#include <glm/glm.hpp>

struct ItemDrop
{
    glm::vec3 position{0.f};
    glm::vec3 velocity{0.f};
    BlockType type = BlockType::Air;
    int count = 1;
    float r = 1.f;
    float g = 1.f;
    float b = 1.f;
    float age = 0.f;
    bool grounded = false;
    bool pickedUp = false;
};
#endif//CAVERN_ITEMDROP_HPP