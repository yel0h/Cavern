#ifndef CAVERN_SERVER_HPP
#define CAVERN_SERVER_HPP
#define WIN32_LEAN_AND_MEAN
#include "NetTypes.hpp"
#include <vector>
#include <winsock2.h>

class Server
{
private:
    struct ClientState
    {
        SOCKET sock = INVALID_SOCKET;
        unsigned int id = 0;
        std::vector<unsigned char> buf;
    };

    SOCKET listenSock = INVALID_SOCKET;
    unsigned int nextId = 1;
    RemotePlayer host{};
    std::vector<ClientState> clients;

    void acceptClients();

    void drainClients();

    void broadcastExcept(const void *data, int len, SOCKET skip);

    void removeClient(int idx);

public:
    std::vector<RemotePlayer> remote;

    bool start(unsigned short port = 5565);

    void shutdown();

    void tick();

    void setHostPos(float x, float y, float z, float yaw);
};
#endif//CAVERN_SERVER_HPP