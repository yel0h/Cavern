#ifndef CAVERN_MOBMANAGER_HPP
#define CAVERN_MOBMANAGER_HPP
#include "Mob.hpp"
#include <random>
#include <vector>

class World;
class Player;
class ParticleManager;

class MobManager
{
private:
    std::mt19937 mt{std::random_device()()};
    std::uniform_real_distribution<float> angleDist{0, 6.28f};

    void pickWanderDir(Mob &m);

    static int surfaceY(const World &world, int wx, int wz);

    static int maxVitalityFor(MobType t);

    static void explode(Mob &m, World &world, ParticleManager &particles);

public:
    std::vector<Mob> mobs;

    void spawn(const World &world);

    void reset() { mobs.clear(); }

    void tick(float dt, World &world, Player &player, ParticleManager &particles);

    bool save(const char *path) const;

    bool load(const char *path);

    bool attack(const glm::vec3 &eye, const glm::vec3 &forward, float reach, int damage, bool &killed, MobType &killedType);

    bool damageMobAt(const glm::vec3 &point, float radius, int damage, bool &killed, MobType &killedType);
};
#endif//CAVERN_MOBMANAGER_HPP