#ifndef CAVERN_SERVER_HPP
#define CAVERN_SERVER_HPP
#define WIN32_LEAN_AND_MEAN
#include "NetTypes.hpp"
#include "Packet.hpp"
#include "src/world/World.hpp"
#include <unordered_map>
#include <vector>
#include <winsock2.h>

class Server
{
private:
    static constexpr int levelChunksPerTick = 8;
    static constexpr int maxClients = 8;
    static constexpr float maxPlaceReach = 6.f;

    struct ClientState
    {
        SOCKET sock = INVALID_SOCKET;
        unsigned int id = 0;
        std::vector<unsigned char> buf;
        char name[16] = {};
        char ip[16] = {};
        bool nameReceived = false;
        int levelSentChunks = -1;
        std::vector<PktBreak> pendingBreakQueue;
    };

    struct ConnRecord
    {
        int count = 0;
    };

    SOCKET listenSock = INVALID_SOCKET;
    unsigned int nextId = 1;
    RemotePlayer host{};
    char hostName[16] = {};
    std::vector<ClientState> clients;
    std::vector<std::string> wardens;
    std::vector<std::string> exiledNames;
    std::vector<std::string> exiledIps;
    std::unordered_map<std::string, ConnRecord> connRecords;
    float lastSentX = 0;
    float lastSentY = 0;
    float lastSentZ = 0;
    float lastSentYaw = 0;
    float spawnX = 128.f;
    float spawnZ = 128.f;
    bool _private = false;

    void acceptClients();

    void drainClients();

    void sendPendingLevels();

    void broadcastExcept(const void *data, int len, SOCKET skip);

    void broadcastBreakExcept(const PktBreak &pk, SOCKET skip);

    void removeClient(int idx);

    void handleCommand(ClientState &sender, const char *raw);

    static void sendServerChat(SOCKET sock, const char *msg);

    bool isWarden(unsigned int id, const char *name) const;

    void loadWardens() { wardens = loadTextList("wardens.txt"); };

    void saveWardens() const { saveTextList("wardens.txt", wardens, "# Cavern warden list"); };

    void loadExiles();

    void saveExiles() const;

    void loadServerSpawn();

    void saveServerSpawn() const;

    void loadConfig();

    void writeExternalUrl(unsigned short port) const;

    void saveLoggedIn() const;

    static std::vector<std::string> loadTextList(const char *path);

    static void saveTextList(const char *path, const std::vector<std::string> &list, const char *header);

public:
    std::vector<RemotePlayer> remote;
    std::vector<BreakEvent> pendingBreaks;
    std::vector<PlaceEvent> pendingPlaces;
    std::vector<ChatEvent> pendingChats;
    const World *world = nullptr;

    bool start(unsigned short port = 5565);

    void shutdown();

    void tick();

    void interpolate(float dt);

    void setHostPos(float x, float y, float z, float yaw);

    void setHostName(const char *n);

    void broadcastBreak(int bx, int by, int bz, unsigned char bt);

    void broadcastChat(unsigned int senderId, const char *msg);

    void handleHostCommand(const char *raw);

    void clearBreaks() { pendingBreaks.clear(); }

    void broadcastPlace(int bx, int by, int bz, unsigned char bt);

    void clearPlaces() { pendingPlaces.clear(); }

    void clearChats() { pendingChats.clear(); }
};
#endif//CAVERN_SERVER_HPP