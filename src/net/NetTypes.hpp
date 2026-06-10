#ifndef CAVERN_NETTYPES_HPP
#define CAVERN_NETTYPES_HPP
struct RemotePlayer
{
    unsigned int id;
    float x;
    float y;
    float z;
    float yaw;
    float vx;
    float vy;
    float vz;
    float vyaw;
    float walkPhase;
};
#endif//CAVERN_NETTYPES_HPP