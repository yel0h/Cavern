#ifndef CAVERN_NETTYPES_HPP
#define CAVERN_NETTYPES_HPP
struct RemotePlayer
{
    unsigned int id{};
    float x{};
    float y{};
    float z{};
    float yaw{};
    float vx{};
    float vy{};
    float vz{};
    float vyaw{};
    float walkPhase{};
    bool posInitialized = false;
    char name[16]{};
};

struct BreakEvent
{
    int bx;
    int by;
    int bz;
    unsigned char blockType;
};

struct ChatEvent
{
    char name[16]{};
    char msg[128]{};
    unsigned char isPrivate = 0;
};

struct LevelChunkEvent
{
    unsigned char cx;
    unsigned char cz;
    unsigned char blocks[16 * 64 * 16];
};
#endif//CAVERN_NETTYPES_HPP