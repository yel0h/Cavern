#ifndef CAVERN_PACKET_HPP
#define CAVERN_PACKET_HPP
#pragma pack(push, 1)
#include "src/world/Chunk.hpp"

enum class PktType : unsigned char
{
    Join = 1,
    Welcome = 2,
    Pos = 3,
    Leave = 4,
    Info = 5,
    Break = 6,
    Chat = 7,
    LevelChunk = 8,
    Expel = 9,
    WardenStatus = 10,
    Spawn = 11,
    Place = 12,
    Warp = 13
};

struct PktJoin
{
    unsigned char type = (unsigned char)PktType::Join;
    char name[16]{};
};

struct PktWelcome
{
    unsigned char type = (unsigned char)PktType::Welcome;
    unsigned char id{};
};

struct PktPos
{
    unsigned char type = (unsigned char)PktType::Pos;
    unsigned char id{};
    float x{};
    float y{};
    float z{};
    float yaw{};
};

struct PktLeave
{
    unsigned char type = (unsigned char)PktType::Leave;
    unsigned char id{};
};

struct PktInfo
{
    unsigned char type = (unsigned char)PktType::Info;
    unsigned int id{};
    char name[16]{};
};

struct PktBreak
{
    unsigned char type = (unsigned char)PktType::Break;
    int bx{};
    int by{};
    int bz{};
    unsigned char blockType{};
};

struct PktChat
{
    unsigned char type = (unsigned char)PktType::Chat;
    unsigned int senderId{};
    char msg[128]{};
    unsigned char isPrivate = 0;
};

struct PktLevelChunk
{
    unsigned char type = (unsigned char)PktType::LevelChunk;
    unsigned char cx{};
    unsigned char cz{};
    unsigned char blocks[Chunk::WIDTH * Chunk::HEIGHT * Chunk::DEPTH]{};
};

struct PktExpel
{
    unsigned char type = (unsigned char)PktType::Expel;
    char reason[64]{};
};

struct PktWardenStatus
{
    unsigned char type = (unsigned char)PktType::WardenStatus;
    unsigned char granted{};
};

struct PktSpawn
{
    unsigned char type = (unsigned char)PktType::Spawn;
    float x{};
    float z{};
};

struct PktPlace
{
    unsigned char type = (unsigned char)PktType::Place;
    int bx{};
    int by{};
    int bz{};
    unsigned char blockType{};
    float px{};
    float py{};
    float pz{};
};

struct PktWarp
{
    unsigned char type=(unsigned char)PktType::Warp;
    float x{};
    float y{};
    float z{};
};
#pragma pack(pop)
#endif//CAVERN_PACKET_HPP