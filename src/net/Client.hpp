#ifndef CAVERN_CLIENT_HPP
#define CAVERN_CLIENT_HPP
#define WIN32_LEAN_AND_MEAN
#include "NetTypes.hpp"
#include <string>
#include <vector>
#include <winsock2.h>

class Client
{
private:
    SOCKET sock = INVALID_SOCKET;
    char localName[16] = {};
    std::vector<unsigned char> buf;
    float lastSentX = 0;
    float lastSentY = 0;
    float lastSentZ = 0;
    float lastSentYaw = 0;

    void drainRecv();

public:
    unsigned int localId = 0;
    std::vector<RemotePlayer> remote;
    std::vector<BreakEvent> pendingBreaks;
    std::vector<PlaceEvent> pendingPlaces;
    std::vector<ChatEvent> pendingChats;
    std::vector<LevelChunkEvent> pendingLevelChunks;
    bool hasPendingSpawn = false;
    float pendingSpawnX = 128.f;
    float pendingSpawnZ = 128.f;

    bool connect(const std::string &host, unsigned short port = 5565);

    void disconnect();

    void tick();

    void sendPosition(float x, float y, float z, float yaw);

    void sendBreak(int bx, int by, int bz, unsigned char bt) const;

    void sendPlace(int bx, int by, int bz, unsigned char bt, float px, float py, float pz);

    void sendChat(const char *msg) const;

    void setLocalName(const char *n);

    void interpolate(float dt);

    [[nodiscard]] bool connected() const { return sock != INVALID_SOCKET; }

    void clearBreaks() { pendingBreaks.clear(); }

    void clearPlaces() { pendingPlaces.clear(); }

    void  clearPendingSpawn() { hasPendingSpawn = false; }

    void clearChats() { pendingChats.clear(); }

    void clearLevelChunks() { pendingLevelChunks.clear(); }
};
#endif//CAVERN_CLIENT_HPP