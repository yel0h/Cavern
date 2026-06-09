#ifndef CAVERN_PACKET_HPP
#define CAVERN_PACKET_HPP
#pragma pack(push, 1)
enum class PktType : unsigned char
{
    Join = 1,
    Welcome = 2,
    Pos = 3,
    Leave = 4
};

struct PktJoin
{
    unsigned char type = (unsigned char)PktType::Join;
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
#pragma pack(pop)
#endif//CAVERN_PACKET_HPP