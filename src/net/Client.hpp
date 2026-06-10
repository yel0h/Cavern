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
    std::vector<unsigned char> buf;

    void drainRecv();

public:
    unsigned int localId = 0;
    char localName[16] = {};
    std::vector<RemotePlayer> remote;
    std::vector<BreakEvent> pendingBreaks;
    std::vector<ChatEvent> pendingChats;

    bool connect(const std::string &host, unsigned short port = 5565);

    void disconnect();

    void tick();

    void sendPosition(float x, float y, float z, float yaw) const;

    void sendBreak(int bx, int by, int bz, unsigned char bt) const;

    void sendChat(const char *msg);

    void setLocalName(const char *n);

    void interpolate(float dt);

    [[nodiscard]] bool connected() const { return sock != INVALID_SOCKET; }

    void clearBreaks() { pendingBreaks.clear(); }

    void clearChats() { pendingChats.clear(); }
};
#endif//CAVERN_CLIENT_HPP