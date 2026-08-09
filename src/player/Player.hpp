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

    struct PlacedEvent
    {
        bool valid = false;
        int bx = 0;
        int by = 0;
        int bz = 0;
        BlockType type = BlockType::Air;
    };

    RayHit hitBlock;
    BrokenEvent lastBroken;
    PlacedEvent lastPlaced;
    BlockType selectedBlock = BlockType::Stone;
    bool placeMode = true;
    bool isWarden = true;
    bool underLava = false;
    bool underWater = false;
    bool justLanded = false;
    bool footstepReady = false;
    BlockType blockBelow = BlockType::Air;
    static constexpr int maxVitality = 16;
    int vitality = maxVitality;
    bool isDown = false;
    float damageFlashT = 0.f;
    float spawnX = 128.f;
    float spawnZ = 128.f;

    [[nodiscard]] glm::vec3 eyePos() const { return position + glm::vec3(0.f, Physics::eye, 0.f); }

    void tick(float dt, World &world, Input &input);

    void applyMouseLook(const Input &input);

    void respawn();

    void applyDamage(int amount);

    void resetSpawn();

    void setSpawn();

    void loadSpawn(float x, float z);

private:
    int footstepTimer = 0;
    float fallPeakY = 0.f;
    float breath = 6.f;
    float drownTimer = 0.f;

    [[nodiscard]] static bool isBlockSolid(const World &world, int wx, int wy, int wz) ;

    [[nodiscard]] RayHit castRay(const World &world) const;

    [[nodiscard]] bool overlapsPlayer(int bx, int by, int bz) const;
};
#endif//CAVERN_PLAYER_HPP