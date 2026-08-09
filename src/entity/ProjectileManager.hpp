#ifndef CAVERN_PROJECTILEMANAGER_HPP
#define CAVERN_PROJECTILEMANAGER_HPP
#include "Bolt.hpp"
#include "Mob.hpp"
#include <vector>
class World;
class MobManager;

class ProjectileManager
{
public:
    std::vector<Bolt> bolts;

    void spawn(glm::vec3 pos, glm::vec3 dir);

    void tick(float dt, World &world, MobManager &mobs, std::vector<MobType> &kills);
};
#endif//CAVERN_PROJECTILEMANAGER_HPP