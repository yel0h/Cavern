#ifndef CAVERN_SERVER_HPP
#define CAVERN_SERVER_HPP
#define WIN32_LEAN_AND_MEAN
#include "NetTypes.hpp"
#include "src/world/World.hpp"
#include <vector>
#include <winsock2.h>

class Server
{
private:
    static constexpr int levelChunksPerTick = 8;

    struct ClientState
    {
        SOCKET sock = INVALID_SOCKET;
        unsigned int id = 0;
        std::vector<unsigned char> buf;
        char name[16] = {};
        bool nameReceived = false;
        int levelSentChunks = -1;
    };

    SOCKET listenSock = INVALID_SOCKET;
    unsigned int nextId = 1;
    RemotePlayer host{};
    char hostName[16] = {};
    std::vector<ClientState> clients;

    void acceptClients();

    void drainClients();

    void sendPendingLevels();

    void broadcastExcept(const void *data, int len, SOCKET skip);

    void removeClient(int idx);

public:
    std::vector<RemotePlayer> remote;
    std::vector<BreakEvent> pendingBreaks;
    std::vector<ChatEvent> pendingChats;
    const World *world = nullptr;

    bool start(unsigned short port = 5565);

    void shutdown();

    void tick();

    void setHostPos(float x, float y, float z, float yaw);

    void setHostName(const char *n);

    void broadcastBreak(int bx, int by, int bz, unsigned char bt);

    void broadcastChat(unsigned int senderId, const char *msg);

    void clearBreaks() { pendingBreaks.clear(); }

    void clearChats() { pendingChats.clear(); }
};
#endif//CAVERN_SERVER_HPP