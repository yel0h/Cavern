#ifndef CAVERN_ITEMDROPMANAGER_HPP
#define CAVERN_ITEMDROPMANAGER_HPP
#include "ItemDrop.hpp"
#include <vector>

class World;

struct PickupEvent
{
    BlockType type;
    int count;
};

class ItemDropManager
{
private:
    std::vector<PickupEvent> pending;

public:
    std::vector<ItemDrop> drops;

    void spawn(glm::vec3 pos, BlockType type, int count);

    void tick(float dt, const World &world, glm::vec3 playerPos);

    std::vector<PickupEvent> drainPickups();
};
#endif//CAVERN_ITEMDROPMANAGER_HPP